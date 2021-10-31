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

// Logging macros
#define LOG_TRACE(...)      Logger::GetLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...)       Logger::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)       Logger::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)      Logger::GetLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...)   Logger::GetLogger()->critical(__VA_ARGS__)