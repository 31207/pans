#ifndef CORE_LOGGER_LOG_RECORD_H
#define CORE_LOGGER_LOG_RECORD_H

#include <chrono>
#include <stdint.h>
#include <pans/logger/log_level.h>
#include <pans/macros.h>
#include <string_view>
namespace pans::detail {
    struct LogRecordView {
        LogLevel::Level m_level = LogLevel::Level::LOG_LV_DEBUG;
        std::string_view m_loggerName = ""; // 日志器名字
        std::string_view m_message = ""; // 日志内容
        std::chrono::system_clock::time_point m_timestamp; // 时间戳
        std::chrono::steady_clock::duration m_elapsed; // 程序启动时长
        u64 m_threadId = 0; // 线程id
        u64 m_fiberId = 0; // 协程id
        std::string_view m_threadName = ""; // 线程名
        std::string_view m_fileName = ""; // 文件名
        u32 m_line = 0;
    };
}

#endif
