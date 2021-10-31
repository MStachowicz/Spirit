#include "Logger.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"

std::shared_ptr<spdlog::logger> Logger::sLogger;

void Logger::init()
{
    spdlog::set_pattern("%^[%T] %n: %v%$");
    sLogger = spdlog::stdout_color_mt("Zeohyr Logger");
    sLogger->set_level(spdlog::level::trace);
}