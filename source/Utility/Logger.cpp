#include "Logger.hpp"

// UI
#include "Editor.hpp"

// STD
#include <iostream>
#include <stdexcept>

void Logger::log_info(const std::string& p_message, const std::source_location& p_location)
{
    const auto info_str = std::format("[INFO] {}\n{}", p_message, to_string(p_location));

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
void Logger::log_warning(const std::string& p_message, const std::source_location& p_location)
{
    const auto warn_str = std::format("[WARNING] {}\n{}", p_message, to_string(p_location));

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
void Logger::log_error(const std::string& p_message, const std::source_location& p_location)
{
    const auto error_str = std::format("[ERROR] {}\n{}", p_message, to_string(p_location));

    if constexpr (s_log_to_editor)
    {
        if (s_editor_sink)
            s_editor_sink->log_error(error_str);
    }

    if constexpr (s_log_to_console)
        std::cout << error_str << std::endl;

    //if constexpr (s_log_to_file)
        // TODO
}
void Logger::assert_fail(const std::string& p_conditional, const std::string& p_message, const std::source_location& p_location)
{
    const auto assert_fail_str = std::format("ASSERT FAILED: '{}' - {}", p_conditional, p_message);
    log_error(assert_fail_str, p_location);
    throw std::logic_error(assert_fail_str);
}

std::string Logger::to_string(const std::source_location& p_location)
{
    return std::format("SOURCE: {} ({}:{})", p_location.function_name(), p_location.file_name(), p_location.line());
}