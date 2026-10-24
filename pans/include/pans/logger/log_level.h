#ifndef INCLUDE_PANS_LOGGER_LOG_LEVEL_H
#define INCLUDE_PANS_LOGGER_LOG_LEVEL_H

#include <cstdint>
#include <string_view>

#include <pans/export.h>

namespace pans {
    class PANS_API LogLevel final {
    public:
        enum class Level : std::uint8_t {
            LOG_LV_DEBUG = 1, // 记录调试信息
            LOG_LV_INFO = 2,
            LOG_LV_WARN = 3,
            LOG_LV_ERROR = 4,
            LOG_LV_FATAL = 5,
            LOG_LV_OFF = 6, // 用于关闭日志
        };

        [[nodiscard]] static PANS_API std::string_view ToString(Level level) noexcept;

        [[nodiscard]] static PANS_API Level FromString(std::string_view value) noexcept;
    };
}

#endif
