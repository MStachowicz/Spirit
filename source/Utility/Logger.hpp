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
	static void log_info(const std::string& p_message);
	static void log_warning(const std::string& p_message, const std::source_location& p_location = std::source_location::current());
	static void log_error(const std::string& p_message, const std::source_location& p_location = std::source_location::current());
	static void assert_fail(const std::string& p_conditional, const std::string& p_message, const std::source_location& p_location = std::source_location::current());
	static inline UI::Editor* s_editor_sink = nullptr; // If pointing to an Editor && s_log_to_editor is true, the editor will output the Log message to its console.

private:
	static constexpr bool s_log_to_file    = false;
	static constexpr bool s_log_to_console = true;
	static constexpr bool s_log_to_editor  = true; // Warning: the editor depends on core and platform work to be done before it can be constructed, thus lots of log messages will not print until it can be assigned later into Spirit initialisation.
	static std::string to_string(const std::source_location& p_location);
};

// Logging is implemented via macros for two reasons.
// 1. The syntax makes very clear the LOG is seperate to the functional code.
// 2. It allows __VA_ARGS__ / parameter packs to be passed as the non-terminal parameter.
//    This is required to make the defaulted source_location::current() param to be at the end.
#ifdef Z_DEBUG
#define LOG(...)       Logger::log_info(std::format(__VA_ARGS__));
#define LOG_WARN(...)  Logger::log_warning(std::format(__VA_ARGS__),                 std::source_location::current());
#define LOG_ERROR(...) Logger::log_error(std::format(__VA_ARGS__),                   std::source_location::current());
#define ASSERT(x, ...) if (!(x)) { Logger::assert_fail(#x, std::format(__VA_ARGS__), std::source_location::current()); }
#else
#define LOG(...)       (void)0;
#define LOG_WARN(...)  (void)0;
#define LOG_ERROR(...) (void)0;
#define ASSERT(x, ...) (void)0;
#endif

#define ASSERT_THROW(x, ...) if (!(x)) { Logger::assert_fail(#x, std::format(__VA_ARGS__), std::source_location::current()); throw std::runtime_error(std::format(__VA_ARGS__)); }


#if defined(_MSC_VER)
	#define DISABLE_WARNING_PUSH           __pragma(warning( push ))
	#define DISABLE_WARNING_POP            __pragma(warning( pop ))
	#define DISABLE_WARNING(warningNumber) __pragma(warning( disable : warningNumber ))

	#define DISABLE_WARNING_UNUSED_VARIABLE            DISABLE_WARNING(4189)
	#define DISABLE_WARNING_UNUSED_PARAMETER           DISABLE_WARNING(4100)
	#define DISABLE_WARNING_UNINITIALIZED
	#define DISABLE_WARNING_HIDES_PREVIOUS_DECLERATION DISABLE_WARNING(4456)

#elif defined(__GNUC__) || defined(__clang__)
	#define DO_PRAGMA(X) _Pragma(#X)
	#define DISABLE_WARNING_PUSH           DO_PRAGMA(GCC diagnostic push)
	#define DISABLE_WARNING_POP            DO_PRAGMA(GCC diagnostic pop)
	#define DISABLE_WARNING(warningName)   DO_PRAGMA(GCC diagnostic ignored #warningName)

	#define DISABLE_WARNING_UNUSED_VARIABLE            DISABLE_WARNING(-Wunused-variable)
	#define DISABLE_WARNING_UNUSED_PARAMETER           DISABLE_WARNING(-Wunused-parameter)
	#define DISABLE_WARNING_UNINITIALIZED              DISABLE_WARNING(-Wuninitialized)
	#define DISABLE_WARNING_HIDES_PREVIOUS_DECLERATION
#else
	#define DISABLE_WARNING_PUSH
	#define DISABLE_WARNING_POP
	#define DISABLE_WARNING_UNUSED_VARIABLE
	#define DISABLE_WARNING_UNUSED_PARAMETER
	#define DISABLE_WARNING_UNINITIALIZED
	#define DISABLE_WARNING_HIDES_PREVIOUS_DECLERATION
#endif
#define UNUSED(...) (void)(__VA_ARGS__);