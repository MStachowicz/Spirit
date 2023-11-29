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
	static void log_info(const std::string& p_message, const std::source_location& p_location = std::source_location::current());
	static void log_warning(const std::string& p_message, const std::source_location& p_location = std::source_location::current());
	static void log_error(const std::string& p_message, const std::source_location& p_location = std::source_location::current());
	static void assert_fail(const std::string& p_conditional, const std::string& p_message, const std::source_location& p_location = std::source_location::current());
	static inline UI::Editor* s_editor_sink = nullptr; // If pointing to an Editor && s_log_to_editor is true, the editor will output the Log message to its console.

private:
	static constexpr bool s_log_to_file    = false;
	static constexpr bool s_log_to_console = true;
	static constexpr bool s_log_to_editor  = true; // Warning: the editor depends on core and platform work to be done before it can be constructed, thus lots of log messages will not print until it can be assigned later into Zephyr initialisation.
	static std::string to_string(const std::source_location& p_location);
};

// Logging is implemented via macros for two reasons.
// 1. The syntax makes very clear the LOG is seperate to the functional code.
// 2. It allows __VA_ARGS__ / parameter packs to be passed as the non-terminal parameter.
//    This is required to make the defaulted source_location::current() param to be at the end.
#ifdef Z_DEBUG
#define LOG(...)       Logger::log_info(std::format(__VA_ARGS__),                    std::source_location::current());
#define LOG_WARN(...)  Logger::log_warning(std::format(__VA_ARGS__),                 std::source_location::current());
#define LOG_ERROR(...) Logger::log_error(std::format(__VA_ARGS__),                   std::source_location::current());
#define ASSERT(x, ...) if (!(x)) { Logger::assert_fail(#x, std::format(__VA_ARGS__), std::source_location::current()); }
#else
#define LOG(...)       (void)0;
#define LOG_WARN(...)  (void)0;
#define LOG_ERROR(...) (void)0;
#define ASSERT(x, ...) (void)0;
#endif