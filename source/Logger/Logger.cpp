#include "Logger.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/wincolor_sink.h"

std::shared_ptr<spdlog::logger> Logger::sLogger;

void Logger::initialise()
{
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_pattern("%^[%H:%M:%S.%e] %v%$");

    sLogger = std::make_shared<spdlog::logger>("Zephyr Logger", consoleSink);
    sLogger->set_level(spdlog::level::trace); // Trace is lowest level = output all log messages
    sLogger->flush_on(spdlog::level::trace);
}