#include "logger/log_record.h"
#include "pans/logger/log_level.h"
#include "pans/macros.h"
#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <cstddef>
#include <ctime>
#include <logger/formatter.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <time.h>
#include <unordered_map>

namespace pans::detail {
constexpr std::string_view DEFAULT_DATE_FORMAT = "%Y-%m-%d %H:%M:%S";
Formatter::Formatter(std::string_view pattern) : m_pattern(pattern) {
    if (parse()) {
        throw std::invalid_argument("invalid log format!");
    }
}

void Formatter::format(const LogRecordView &record,
                       FormattedRecordBuffer &output) const {
    for (const auto &item : m_items) {
        item->format(record, output);
    }
}

template <typename T>
void AppendInteger(FormattedRecordBuffer &output, T value) {
    std::array<char, 24> buffer{};
    const auto RESULT =
        std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);
    ASSERT_RETNONE2(RESULT.ec == std::errc(),
                    "trans" << value << " to chars failed.");
    output.append(buffer.data(),
                  static_cast<std::size_t>(RESULT.ptr - buffer.data()));
}

class LiteralFormatItem final : public Formatter::FormatItem {
public:
    explicit LiteralFormatItem(std::string_view value) : m_value(value) {}
    void format(const LogRecordView &,
                FormattedRecordBuffer &output) const override {
        output.append(m_value);
    }

private:
    std::string m_value;
};
class MessageFormatItem final : public Formatter::FormatItem {
public:
    void format(const LogRecordView &record,
                FormattedRecordBuffer &output) const override {
        output.append(record.m_message);
    }
};

class LevelFormatItem final : public Formatter::FormatItem {
public:
    void format(const LogRecordView &record,
                FormattedRecordBuffer &output) const override {
        output.append(LogLevel::ToString(record.m_level));
    }
};
class ElapsedFormatItem final : public Formatter::FormatItem {
public:
    void format(const LogRecordView &record,
                FormattedRecordBuffer &output) const override {
        AppendInteger(output,
                      std::chrono::duration_cast<std::chrono::milliseconds>(
                          record.m_elapsed)
                          .count());
    }
};
class LoggerNameFormatItem final : public Formatter::FormatItem {
public:
    void format(const LogRecordView &record,
                FormattedRecordBuffer &output) const override {
        output.append(record.m_loggerName);
    }
};
class ThreadIdFormatItem final : public Formatter::FormatItem {
public:
    void format(const LogRecordView &record,
                FormattedRecordBuffer &output) const override {
        AppendInteger(output, record.m_threadId);
    }
};
class NewLineFormatItem final : public Formatter::FormatItem {
public:
    void format(const LogRecordView &,
                FormattedRecordBuffer &output) const override {
        output.append('\n');
    }
};
class DateTImeFormatItem final : public Formatter::FormatItem {
public:
    explicit DateTImeFormatItem() : m_format(DEFAULT_DATE_FORMAT) {}
    explicit DateTImeFormatItem(std::string_view format) : m_format(format) {}
    void format(const LogRecordView &record,
                FormattedRecordBuffer &output) const override {
        static thread_local i32 s_lastSecond = 0;
        static thread_local char s_cachedDateTime[20] = {'\0'};
        const auto DURATION = record.m_timestamp.time_since_epoch();
        const time_t CURRENT_SECOND = static_cast<time_t>(
            std::chrono::duration_cast<std::chrono::seconds>(DURATION).count());

        if (CURRENT_SECOND != s_lastSecond) {
            std::tm buffer{};
#if defined(_WIN32)
            const errno_t result = localtime_s(&buffer, &CURRENT_SECOND);
            ASSERT_NOEFFECT2(result == 0,
                             "failed to convert log time to local time");
#else
            const std::tm *result = localtime_r(&CURRENT_SECOND, &buffer);
            ASSERT_NOEFFECT2(result != nullptr,
                             "failed to convert log time to local time");
#endif
            const std::size_t SIZE =
                std::strftime(s_cachedDateTime, sizeof(s_cachedDateTime),
                              m_format.c_str(), &buffer);
            ASSERT_RETNONE2(SIZE != 0, "failed to format log time");
            s_lastSecond = CURRENT_SECOND;
            output.append(s_cachedDateTime);
        }
    }

private:
    std::string m_format;
};

class MicrosecondsFormatItem final : public Formatter::FormatItem {
public:
    void format(const LogRecordView &record,
                FormattedRecordBuffer &output) const override {
        const auto TOTAL_MICROSECONDS =
            std::chrono::duration_cast<std::chrono::microseconds>(
                record.m_timestamp.time_since_epoch())
                .count();
        i64 microseconds = TOTAL_MICROSECONDS % 1000000;
        if (microseconds) {
            microseconds += 1000000;
        }
        // 对齐微秒的显示, eg: 01:22:33.007896
        std::array<char, MICROSECONDS_WIDTH> digits{};
        auto remaining = static_cast<u32>(microseconds);
        for (std::size_t i = digits.size(); i-- > 0;) {
            digits[i] = static_cast<char>('0' + remaining % 10);
            remaining /= 10;
        }
        output.append(digits.data(), digits.size());
    }

private:
    static constexpr std::size_t MICROSECONDS_WIDTH = 6;
};
class FileNameFormatItem final : public Formatter::FormatItem {
public:
    void format(const LogRecordView &record,
                FormattedRecordBuffer &output) const override {
        output.append(record.m_fileName);
    }
};
class LineFormatItem final : public Formatter::FormatItem {
public:
    void format(const LogRecordView &record,
                FormattedRecordBuffer &output) const override {
        AppendInteger(output, record.m_line);
    }
};
class ThreadNameFormatItem final : public Formatter::FormatItem {
public:
    void format(const LogRecordView &record,
                FormattedRecordBuffer &output) const override {
        output.append(record.m_threadName);
    }
};
class FiberIdFormatItem final : public Formatter::FormatItem {
public:
    void format(const LogRecordView &record,
                FormattedRecordBuffer &output) const override {
        AppendInteger(output, record.m_fiberId);
    }
};
class TabFormatItem final : public Formatter::FormatItem {
public:
    void format(const LogRecordView &,
                FormattedRecordBuffer &output) const override {
        output.append('\t');
    }
};

using FormatItemFactory =
    std::unique_ptr<Formatter::FormatItem> (*)(std::string_view);

template <typename Item>
[[nodiscard]] std::unique_ptr<Formatter::FormatItem>
CreateSimpleFormatItem(std::string_view) {
    return std::make_unique<Item>();
}
template <typename Item>
[[nodiscard]] std::unique_ptr<Formatter::FormatItem>
CreateConfiguedFormatItem(std::string_view format) {
    return std::make_unique<Item>(format);
}

[[nodiscard]] const std::unordered_map<char, FormatItemFactory> &
GetFormatItemFactories() {
    static const std::unordered_map<char, FormatItemFactory>
        FORMAT_ITEM_FACTORIES{
            {'m', &CreateSimpleFormatItem<MessageFormatItem>},
            {'p', &CreateSimpleFormatItem<LevelFormatItem>},
            {'r', &CreateSimpleFormatItem<ElapsedFormatItem>},
            {'c', &CreateSimpleFormatItem<LoggerNameFormatItem>},
            {'t', &CreateSimpleFormatItem<ThreadIdFormatItem>},
            {'n', &CreateSimpleFormatItem<NewLineFormatItem>},
            {'d', &CreateSimpleFormatItem<DateTImeFormatItem>},
            {'u', &CreateSimpleFormatItem<MicrosecondsFormatItem>},
            {'f', &CreateSimpleFormatItem<FileNameFormatItem>},
            {'l', &CreateSimpleFormatItem<LineFormatItem>},
            {'T', &CreateSimpleFormatItem<TabFormatItem>},
            {'F', &CreateSimpleFormatItem<FiberIdFormatItem>},
            {'N', &CreateSimpleFormatItem<ThreadNameFormatItem>},
        };
    return FORMAT_ITEM_FACTORIES;
}

[[nodiscard]] std::unique_ptr<Formatter::FormatItem>
CreateFormatItem(char directive, std::string_view format) {
    const auto &FACTORIES = GetFormatItemFactories();
    const auto it = FACTORIES.find(directive);
    ASSERT_RETVAL2(it != FACTORIES.end(), nullptr,
                   "unknown logger format directive: " << directive);
    return it->second(format);
}

void Formatter::addLiteral(std::string &literal) {
    if (literal.empty()) {
        return;
    }
    m_items.emplace_back(
        std::make_unique<LiteralFormatItem>(std::move(literal)));
    literal.clear();
}

int Formatter::parse() {
    std::string literal;
    for (std::size_t i = 0; i < m_pattern.size(); ++i) {
        if (m_pattern[i] != '%') {
            literal.push_back(m_pattern[i]);
            continue;
        }
        ASSERT_RETVAL2(
            i + 1 < m_pattern.size(), -1,
            "logger format pattern ends with an incomplete directive");
        const char DIRECITVE = m_pattern[++i];
        if (DIRECITVE == '%') {
            literal.push_back('%');
            continue;
        }
        addLiteral(literal);
        std::string_view item_format;
        if (i + 1 < m_pattern.size() && m_pattern[i + 1] == '{') {
            const std::size_t CLOSING_BRACE = m_pattern.find('}', i + 2);
            ASSERT_RETVAL2(CLOSING_BRACE != std::string::npos &&
                               CLOSING_BRACE != (i + 2),
                           -2, "missing a closing brace or empty");
            item_format = std::string_view(m_pattern).substr(
                i + 2, CLOSING_BRACE - i - 2);
            i = CLOSING_BRACE;
        }
        m_items.push_back(CreateFormatItem(DIRECITVE, item_format));
    }
    addLiteral(literal);
    for (const auto& item : m_items) {
        ASSERT_RETVAL2(item !=nullptr, -3, "logger formatter contains a null format item");
    }
    return 0;
}

} // namespace pans::detail
