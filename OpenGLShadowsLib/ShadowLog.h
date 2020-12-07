#pragma once

#include "spdlog/spdlog.h"

#define SHADOW_LEVEL_TRACE 0
#define SHADOW_LEVEL_DEBUG 1
#define SHADOW_LEVEL_INFO 2
#define SHADOW_LEVEL_WARN 3
#define SHADOW_LEVEL_ERROR 4
#define SHADOW_LEVEL_CRITICAL 5
#define SHADOW_LEVEL_OFF 6

#ifndef SHADOW_LOG_LEVEL
#define SHADOW_LOG_LEVEL SHADOW_LEVEL_DEBUG
#endif

#define SHADOW_LOGGER_CALL(logger, level, ...) (logger)->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, level, __VA_ARGS__)

#if SHADOW_LOG_LEVEL <= SHADOW_LEVEL_TRACE
#define SHADOW_LOGGER_TRACE(logger, ...) SHADOW_LOGGER_CALL(shadow::ShadowLog::getLogger(), spdlog::level::trace, __VA_ARGS__)
#define SHADOW_TRACE(...) SHADOW_LOGGER_TRACE(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define SHADOW_LOGGER_TRACE(logger, ...) (void)0
#define SHADOW_TRACE(...) (void)0
#endif

#if SHADOW_LOG_LEVEL <= SHADOW_LEVEL_DEBUG
#define SHADOW_LOGGER_DEBUG(logger, ...) SHADOW_LOGGER_CALL(shadow::ShadowLog::getLogger(), spdlog::level::debug, __VA_ARGS__)
#define SHADOW_DEBUG(...) SHADOW_LOGGER_DEBUG(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define SHADOW_LOGGER_DEBUG(logger, ...) (void)0
#define SHADOW_DEBUG(...) (void)0
#endif

#if SHADOW_LOG_LEVEL <= SHADOW_LEVEL_INFO
#define SHADOW_LOGGER_INFO(logger, ...) SHADOW_LOGGER_CALL(shadow::ShadowLog::getLogger(), spdlog::level::info, __VA_ARGS__)
#define SHADOW_INFO(...) SHADOW_LOGGER_INFO(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define SHADOW_LOGGER_INFO(logger, ...) (void)0
#define SHADOW_INFO(...) (void)0
#endif

#if SHADOW_LOG_LEVEL <= SHADOW_LEVEL_WARN
#define SHADOW_LOGGER_WARN(logger, ...) SHADOW_LOGGER_CALL(shadow::ShadowLog::getLogger(), spdlog::level::warn, __VA_ARGS__)
#define SHADOW_WARN(...) SHADOW_LOGGER_WARN(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define SHADOW_LOGGER_WARN(logger, ...) (void)0
#define SHADOW_WARN(...) (void)0
#endif

#if SHADOW_LOG_LEVEL <= SHADOW_LEVEL_ERROR
#define SHADOW_LOGGER_ERROR(logger, ...) SHADOW_LOGGER_CALL(shadow::ShadowLog::getLogger(), spdlog::level::err, __VA_ARGS__)
#define SHADOW_ERROR(...) SHADOW_LOGGER_ERROR(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define SHADOW_LOGGER_ERROR(logger, ...) (void)0
#define SHADOW_ERROR(...) (void)0
#endif

#if SHADOW_LOG_LEVEL <= SHADOW_LEVEL_CRITICAL
#define SHADOW_LOGGER_CRITICAL(logger, ...) SHADOW_LOGGER_CALL(shadow::ShadowLog::getLogger(), spdlog::level::critical, __VA_ARGS__)
#define SHADOW_CRITICAL(...) SHADOW_LOGGER_CRITICAL(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define SHADOW_LOGGER_CRITICAL(logger, ...) (void)0
#define SHADOW_CRITICAL(...) (void)0
#endif

namespace shadow
{
    class ShadowLog sealed
    {
    public:
        ShadowLog() = delete;
        static spdlog::logger* getLogger();
    private:
        static std::shared_ptr<spdlog::logger> logger;
        static const std::string SHADOW_LOG_NAME;
        static const std::string SHADOW_LOG_FILENAME;
        static const size_t SHADOW_LOG_FILE_SIZE;
        static const size_t SHADOW_LOG_MAX_FILES;
    };
}