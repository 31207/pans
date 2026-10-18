#include "logger/log_record.h"
#include "pans/logger/log_level.h"
#include "pans/macros.h"
#include <array>
#include <charconv>
#include <chrono>
#include <logger/formatter.h>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

namespace pans::detail {
constexpr std::string_view DEFAULT_DATE_FORMAT = "%Y-%m-%d %H:%M:%S";
Formatter::Formatter(std::string_view pattern) : m_pattern(pattern) {
    if (parse()) {
        throw std::invalid_argument("invalid log format!");
    }
}

void Formatter::format(const LogRecordView &record,
                       FormattedRecordBuffer &output) const noexcept {
    for (const auto &item : m_items) {
        item->format(record, output);
    }
}

template <typename T>
void AppendInteger(FormattedRecordBuffer &output, T value) noexcept {
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
    void format(const LogRecordView &_,
                FormattedRecordBuffer &output) const noexcept {
        output.append(m_value);
    }
private:
    std::string m_value;
};
class MessageFormatItem final : public Formatter::FormatItem {
public:
    void format(const LogRecordView &record,
                FormattedRecordBuffer &output) const noexcept {
        output.append(record.m_message);
    }
};

class LevelFormatItem final : public Formatter::FormatItem {
public:
    void format(const LogRecordView &record,
                FormattedRecordBuffer &output) const noexcept {
        output.append(LogLevel::ToString(record.m_level));
    }
};
class ElapsedFormatItem final : public Formatter::FormatItem {
public:
    void format(const LogRecordView &record,
                FormattedRecordBuffer &output) const noexcept {
        AppendInteger(output, std::chrono::duration_cast<std::chrono::milliseconds>(record.m_elapsed).count());
    }
};
class LoggerNameFormatItem final : public Formatter::FormatItem {
public:
    void format(const LogRecordView &record,
                FormattedRecordBuffer &output) const noexcept {
        output.append(record.m_loggerName);
    }
};
class ThreadIdFormatItem final : public Formatter::FormatItem {
public:
    void format(const LogRecordView &record,
                FormattedRecordBuffer &output) const noexcept {
        AppendInteger(output, record.m_threadId);
    }
};
class NewLineFormatItem final : public Formatter::FormatItem {
public:
    void format(const LogRecordView &_,
                FormattedRecordBuffer &output) const noexcept {
        output.append('\n');
    }
};
class DateTImeFormatItem final : public Formatter::FormatItem {
public:
    void format(const LogRecordView &record,
                FormattedRecordBuffer &output) const noexcept {
        // todo: 格式化日期并添加到日志中
    }
};

} // namespace pans::detail
