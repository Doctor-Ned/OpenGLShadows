#include "ShadowLog.h"

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

std::shared_ptr<spdlog::logger> shadow::ShadowLog::logger{};
const std::string shadow::ShadowLog::SHADOW_LOG_NAME{ "Shadows" };
const std::string shadow::ShadowLog::SHADOW_LOG_FILENAME{ "shadow.log" };
const size_t shadow::ShadowLog::SHADOW_LOG_FILE_SIZE{ 8'388'608UL };
const size_t shadow::ShadowLog::SHADOW_LOG_MAX_FILES{ 3UL };

spdlog::logger* shadow::ShadowLog::getLogger()
{
    if (!logger)
    {
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto rotatingFileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(SHADOW_LOG_FILENAME, SHADOW_LOG_FILE_SIZE, SHADOW_LOG_MAX_FILES);
        spdlog::sinks_init_list sinkList{ consoleSink, rotatingFileSink };
        logger = std::make_shared<spdlog::logger>(SHADOW_LOG_NAME, sinkList);
        logger->set_level(static_cast<spdlog::level::level_enum>(SHADOW_LOG_LEVEL));
        set_default_logger(logger);
    }
    return logger.get();
}
