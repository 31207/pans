#ifndef CORE_LOGGER_FORMATTER_H
#define CORE_LOGGER_FORMATTER_H

#include <logger/buffer.h>
#include <logger/buffer_config.h>
#include <logger/log_record.h>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace pans::detail {
using FormattedRecordBuffer = InlineBuffer<FORMATTED_RECORD_INLINE_CAPACITY>;
class Formatter final {
public:
    class FormatItem {
    public:
        virtual ~FormatItem() = default;
        virtual void format(const LogRecordView &record,
                            FormattedRecordBuffer &out) const noexcept = 0;
    };
    explicit Formatter(std::string_view pattern);
    void format(const LogRecordView &record,
                FormattedRecordBuffer &output) const noexcept;
    [[nodiscard]] const std::string &getPattern() const noexcept {
        return m_pattern;
    };

private:
    int parse() noexcept;
    void addLiteral(std::string literal) noexcept;

    std::string m_pattern = "";
    std::vector<std::unique_ptr<FormatItem>> m_items;
};
} // namespace pans::detail

#endif
