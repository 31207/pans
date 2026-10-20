#ifndef CORE_LOGGER_APPENDER_IMPL_H
#define CORE_LOGGER_APPENDER_IMPL_H
#include "pans/logger/log_level.h"
#include <atomic>
#include <cstdio>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

#include <pans/logger/appender.h>
namespace pans {
class Appender::Impl {
public:
    virtual ~Impl() = default;
    void append(LogLevel::Level level, std::string_view formatted_record) noexcept;
    void setLevel(LogLevel::Level level) noexcept;
    [[nodiscard]] LogLevel::Level getLevel() const noexcept;
    void flush();
    void sync();

protected:
    virtual void writeUnlocked(std::string_view formatted_record) noexcept = 0;
    virtual void flushUnlocked() noexcept = 0;
    virtual void syncUnlocked() noexcept = 0;

    std::mutex m_mutex;

private:
    std::atomic<LogLevel::Level> m_level{LogLevel::Level::LOG_LV_DEBUG};
};

namespace detail {
using AppenderPtr = std::shared_ptr<Appender>;

class AppenderAccess final {
public:
    [[nodiscard]] static AppenderPtr MakeStdoutAppender();
    [[nodiscard]] static AppenderPtr MakeFileAppender(std::string file_name);
    static void Append(const AppenderPtr& appender, LogLevel::Level level, std::string_view formatted_record) noexcept;
};
}

}
#endif
