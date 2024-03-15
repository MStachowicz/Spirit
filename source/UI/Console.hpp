#pragma once

#include "glm/vec3.hpp"

#include <string>
#include <vector>

struct ImGuiInputTextCallbackData;

namespace UI
{
	// Console with an input and filter view for outputting application messages to the editor.
	// The input box supports commands making this a command-line-esque console.
	struct Console
	{
		struct Message
		{
			std::string m_message;
			glm::vec3 m_colour;

			Message(const std::string& p_message, const glm::vec3& p_colour)
				: m_message{p_message}
				, m_colour{p_colour}
			{}
			Message(const std::string& p_message);
		};

	private:
		std::vector<std::string> m_commands; // Available commands in the input box
		std::vector<std::string> m_history;
		std::vector<Message> m_log_messages;
		std::string m_input;
		bool m_scroll_to_bottom; // On the next draw, scroll the scrollable message region to the bottom and stay there.

	public:
		Console();
		~Console() = default;

		void clear_log()                       { m_log_messages.clear(); }
		void add_log(const Message& p_message) { m_log_messages.push_back(p_message); }
		void draw(const char* title, bool* p_open);

	private:
		void execute_command(const std::string& p_command);
		int text_edit_callback(ImGuiInputTextCallbackData* data);
	};
}