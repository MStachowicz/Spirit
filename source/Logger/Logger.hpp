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
#define ZEPHYR_ASSERT(x, ...) if ((x)) {} else { ZephyrAssertImplementation(#x, __FILE__, __LINE__, __VA_ARGS__); }
#define ZEPHYR_ASSERT_MSG(message) const std::string errorMessage = message; if ((errorMessage.empty())) {} else { ZephyrAssertMessageImplementation(errorMessage, __FILE__, __LINE__); }

template <typename ...Args>
void ZephyrAssertImplementation(const std::string& pCondition, const char* pFile, const int& pLine, Args&& ...pArgs)
{
    // Parses all the params and outputs them via SPDLOG
    // #C++20 - Migrate the fmt::format to std::format
    //        - use std::source_location (non-macro function name)
    // Once migrated to std::format, additional std types get formatting e.g. std::filesystem::path
    // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1636r0.pdf
    const std::string message = fmt::format(std::forward<Args>(pArgs)...);
    Logger::GetLogger()->critical(fmt::format("ASSERT FAILED\nMESSAGE:   {}\nCONDITION: {}\nFILE: {}:{}", message, pCondition, pFile, pLine));
    throw std::logic_error(message);
}

inline void ZephyrAssertMessageImplementation(const std::string& pErrorMessage, const char* pFile, const int& pLine)
{
    // Parses all the params and outputs them via SPDLOG
    // #C++20 - Migrate the fmt::format to std::format
    //        - use std::source_location (non-macro function name)
    // Once migrated to std::format, additional std types get formatting e.g. std::filesystem::path
    // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1636r0.pdf
    Logger::GetLogger()->critical(fmt::format("ASSERT FAILED\nMESSAGE: {}\nFILE:    {}:{}", pErrorMessage, pFile, pLine));
    throw std::logic_error(pErrorMessage);
}
#else
// Disable logging for release builds
#define LOG_TRACE(...)             (void)0
#define LOG_INFO(...)              (void)0
#define LOG_WARN(...)              (void)0
#define LOG_ERROR(...)             (void)0
#define LOG_CRITICAL(...)          (void)0
#define ZEPHYR_ASSERT(x, ...)      (void)0
#define ZEPHYR_ASSERT_MSG(message) (void)0

#endif