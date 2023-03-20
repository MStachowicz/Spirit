#pragma once

#include <string>
#include <format>
#include <source_location>

namespace UI
{
    class Editor;
}

class Logger
{
public:
    void log_info(const std::string& p_message, const std::source_location& p_location = std::source_location::current());
    void log_warning(const std::string& p_message, const std::source_location& p_location = std::source_location::current());
    void log_error(const std::string& p_message, const std::source_location& p_location = std::source_location::current());
    void assert_fail(const std::string& p_conditional, const std::string& p_message, const std::source_location& p_location = std::source_location::current());
    UI::Editor* m_editor_sink; // If pointing to an Editor && s_log_to_editor is true, the editor will output the Log message to its console.
private:
    static constexpr bool s_log_to_file    = false;
    static constexpr bool s_log_to_console = false;
    static constexpr bool s_log_to_editor  = true;

    static std::string to_string(const std::source_location& p_location);
};
static Logger s_logger; // Global instance of logger, prefer using macros to call Log functions (LOG, LOG_WARN, LOG_ERROR)

// Logging is implemented via macros for two reasons.
// 1. The syntax makes very clear the LOG is seperate to the functional code.
// 2. It allows __VA_ARGS__ / parameter packs to be passed as the non-terminal parameter.
//    This is required to make the defaulted source_location::current() param to be at the end.
#ifndef RELEASE
#define LOG(...)       s_logger.log_info(std::format(__VA_ARGS__),                  std::source_location::current());
#define LOG_WARN(...)  s_logger.log_warning(std::format(__VA_ARGS__),               std::source_location::current());
#define LOG_ERROR(...) s_logger.log_error(std::format(__VA_ARGS__),                 std::source_location::current());
#define ASSERT(x, ...) if (!(x)) { s_logger.assert_fail(#x, std::format(__VA_ARGS__), std::source_location::current()); }
#else
#define LOG(...)       (void)0
#define LOG_WARN(...)  (void)0
#define LOG_ERROR(...) (void)0
#define ASSERT(x, ...) (void)0
#endif