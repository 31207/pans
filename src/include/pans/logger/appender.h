#ifndef INCLUDE_PANS_LOGGER_APPENDER_H
#define INCLUDE_PANS_LOGGER_APPENDER_H

#include <memory>

#include <pans/export.h>
#include <pans/logger/log_level.h>
#include <string>

namespace pans {
namespace detail {
class AppenderAccess;
}

class PANS_API Appender final {
public:
    class Impl;
    ~Appender() =default;
    Appender(const Appender&) = delete;
    Appender& operator=(const Appender&) = delete;
    Appender(Appender&&) = delete;
    Appender& operator=(Appender&&) = delete;

    void setLevel(LogLevel::Level level) noexcept;
    [[nodiscard]] LogLevel::Level getLevel() const noexcept;

    void flush(); // 将用户态内存中的内容提交到内核
    void sync();  // 要求操作系统将内核中的日志提交到磁盘
private:
    explicit Appender(std::unique_ptr<Impl> impl) noexcept;
    std::unique_ptr<Impl> m_impl;
    friend class detail::AppenderAccess;
};

using AppenderPtr = std::shared_ptr<Appender>;

[[nodiscard]] PANS_API AppenderPtr MakeStdoutAppender();
[[nodiscard]] PANS_API AppenderPtr MakeFileAppender(std::string file_name);
// 以后也可以开发向远程提交日志的功能

} // namespace pans

#endif
