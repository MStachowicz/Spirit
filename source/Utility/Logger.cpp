#include "Logger.hpp"

#include "UI/Editor.hpp"

#include <iostream>
#include <stdexcept>

void Logger::log_info(std::string_view p_message)
{
	const auto info_str = std::format("[INFO] {}", p_message);

	if constexpr (s_log_to_editor)
	{
		if (s_editor_sink)
			s_editor_sink->log(info_str);
	}

	if constexpr (s_log_to_console)
		std::cout << info_str << std::endl;

	//if constexpr (s_log_to_file)
		// TODO
}
void Logger::log_warning(std::string_view p_message, const std::source_location& p_location)
{
	const auto warn_str = std::format("[WARNING] {}\n{} ({}:{})", p_message, p_location.function_name(), p_location.file_name(), p_location.line());

	if constexpr (s_log_to_editor)
	{
		if (s_editor_sink)
			s_editor_sink->log_warning(warn_str);
	}

	if constexpr (s_log_to_console)
		std::cout << warn_str << std::endl;

	//if constexpr (s_log_to_file)
		// TODO
}
void Logger::log_error(std::string_view p_message, const std::source_location& p_location)
{
	const auto error_str = std::format("[ERROR] {}\n{} ({}:{})", p_message, p_location.function_name(), p_location.file_name(), p_location.line());

	if constexpr (s_log_to_editor)
	{
		if (s_editor_sink)
			s_editor_sink->log_error(error_str);
	}

	if constexpr (s_log_to_console)
		std::cerr << error_str << std::endl;

	//if constexpr (s_log_to_file)
		// TODO
}
void Logger::log_warning_no_location(std::string_view p_message)
{
	const auto warn_str = std::format("[WARNING] {}", p_message);

	if constexpr (s_log_to_editor)
	{
		if (s_editor_sink)
			s_editor_sink->log_warning(warn_str);
	}

	if constexpr (s_log_to_console)
		std::cout << warn_str << std::endl;

	//if constexpr (s_log_to_file)
		// TODO
}
void Logger::log_error_no_location(std::string_view p_message)
{
	const auto error_str = std::format("[ERROR] {}", p_message);

	if constexpr (s_log_to_editor)
	{
		if (s_editor_sink)
			s_editor_sink->log_error(error_str);
	}

	if constexpr (s_log_to_console)
		std::cerr << error_str << std::endl;

	//if constexpr (s_log_to_file)
		// TODO
}
[[noreturn]] void Logger::assert_fail(std::string_view p_conditional, std::string_view p_message, const std::source_location& p_location)
{
	const auto assert_fail_str = std::format("ASSERT FAILED: '{}' - {}", p_conditional, p_message);
	log_error(assert_fail_str, p_location);
	throw std::logic_error(assert_fail_str);
}
[[noreturn]] void Logger::assert_fail(std::string_view p_message, const std::source_location& p_location)
{
	const auto assert_fail_str = std::format("ASSERT FAILED: '{}'", p_message);
	log_error(assert_fail_str, p_location);
	throw std::logic_error(assert_fail_str);
}