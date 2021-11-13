#include "spdlog/spdlog.h"

// Static logger using spdlog via macros
class Logger
{
public:
    static void init();
    inline static std::shared_ptr<spdlog::logger> &GetLogger() { return sLogger; }

private:
    static std::shared_ptr<spdlog::logger> sLogger;
};


#ifdef ZEPHYR_CONFIG_DEBUG
// Logging macros
#define LOG_TRACE(...)      Logger::GetLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...)       Logger::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)       Logger::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)      Logger::GetLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...)   Logger::GetLogger()->critical(__VA_ARGS__)
#else if ZEPHYR_CONFIG_RELEASE
// Disable logging for release builds
#define LOG_TRACE(...)      (void)0   
#define LOG_INFO(...)       (void)0
#define LOG_WARN(...)       (void)0
#define LOG_ERROR(...)      (void)0
#define LOG_CRITICAL(...)   (void)0
#endif