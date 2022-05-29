#pragma once

#include "spdlog/spdlog.h"
#include "stdexcept"

// Static logger using spdlog via macros
class Logger
{
public:
    static void initialise();
    inline static std::shared_ptr<spdlog::logger>& GetLogger() { return sLogger; }

private:
    static std::shared_ptr<spdlog::logger> sLogger;
};

#ifndef ZEPHYR_CONFIG_RELEASE
// Logging macros
#define LOG_TRACE(...)      Logger::GetLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...)       Logger::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)       Logger::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)      Logger::GetLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...)   Logger::GetLogger()->critical(__VA_ARGS__)
#define ZEPHYR_ASSERT(x, ...) if ((x)) {} else { ZephyrAssertImplementation(#x, __LINE__, __FILE__, __VA_ARGS__); }

template <typename ...Args>
void ZephyrAssertImplementation(const std::string& condition, const int& line, const char* fileName, Args&& ...args)
{
    // Parses all the params and outputs them via SPDLOG
    // #C++20 - Migrate the fmt::format to std::format
    // Once migrated to std::format, additional std types get formatting e.g. std::filesystem::path
    // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1636r0.pdf
    const std::string message = fmt::format(std::forward<Args>(args)...);
    Logger::GetLogger()->critical(fmt::format("ASSERT FAILED\nMESSAGE:   {}\nCONDITION: {}\nFILE:      {}\nLINE:      {}", message, condition, fileName, line));
    throw std::logic_error(message);
}
#else
// Disable logging for release builds
#define LOG_TRACE(...)          (void)0
#define LOG_INFO(...)           (void)0
#define LOG_WARN(...)           (void)0
#define LOG_ERROR(...)          (void)0
#define LOG_CRITICAL(...)       (void)0
#define ZEPHYR_ASSERT(x, ...)   (void)0
#endif