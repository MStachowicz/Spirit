#include "Console.hpp"
#include "Platform/Core.hpp"

#include "imgui.h"

#include <format>
#include <algorithm>

namespace UI
{
	Console::Message::Message(const std::string& p_message)
		: m_message{p_message}
		, m_colour{Platform::Core::s_theme.general_text}
	{}

	Console::Console()
		: m_commands{"HELP", "HISTORY", "CLEAR", "CLASSIFY"}
		, m_history{}
		, m_log_messages{}
		, m_input{""}
		, m_scroll_to_bottom{false}
	{}

	void Console::draw(const char* title, bool* p_open)
	{
		static ImGuiTextFilter m_filter; // Declared statically to avoid having to include ImGui in the header.
		if (ImGui::Begin(title, p_open))
		{
			bool copy_to_clipboard = false;

			{ // The consoleHeader is drawn in a table to allow the filter to fill the remaining space and be seperated from the options in the first column.
				ImGui::Separator();
				if (ImGui::BeginTable("ConsoleHeader", 2, ImGuiTableFlags_Resizable))
				{
					ImGui::TableNextRow();
					{
						ImGui::TableSetColumnIndex(0);
						if (ImGui::Button("Copy")) copy_to_clipboard = true;
						ImGui::SameLine();
						if (ImGui::Button("Clear")) clear_log();
						ImGui::SameLine();
						if (ImGui::ArrowButton("Scroll to bottom", ImGuiDir_Down)) m_scroll_to_bottom = true;
					}
					{
						ImGui::TableSetColumnIndex(1);
						ImGui::Text("Filter");
						ImGui::SameLine();
						m_filter.Draw("##ConsoleFilter", ImGui::GetContentRegionAvail().x);
					}

					ImGui::EndTable();
				}
				ImGui::Separator();
			}

			{ // The scrollable region where all the messages are printed.
				const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing(); // space for the command-line footer
				ImGui::BeginChild("ConsoleScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing

				if (copy_to_clipboard) ImGui::LogToClipboard();
				for (size_t i = 0; i < m_log_messages.size(); i++)
				{
					const char* item = m_log_messages[i].m_message.c_str();
					if (!m_filter.PassFilter(item))
						continue;

					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(m_log_messages[i].m_colour.r, m_log_messages[i].m_colour.g, m_log_messages[i].m_colour.b, 1.0f));
					ImGui::TextUnformatted(m_log_messages[i].m_message.c_str());
					ImGui::PopStyleColor();
				}
				if (copy_to_clipboard) ImGui::LogFinish();

				// If m_scroll_to_bottom is selected or we have scrolled to the bottom, we fix the scrollingRegion there.
				if (m_scroll_to_bottom || (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
				{
					ImGui::SetScrollHereY(1.0f);
					m_scroll_to_bottom = false;
				}

				ImGui::PopStyleVar();
				ImGui::EndChild();
			}

			{ // Command-line
				const ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
				bool reclaim_focus = false;
				char buffer[256] = "";

				// ImGui::InputText requires a callback
				auto text_edit_callback_stub = [](ImGuiInputTextCallbackData* p_data)
				{
					Console* console = (Console*)p_data->UserData;
					console->m_input = p_data->Buf;
					return console->text_edit_callback(p_data);
				};

				ImGui::Separator();
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

				if (ImGui::InputText("##ConsoleCommandLineInput", buffer, sizeof(buffer), flags, text_edit_callback_stub, (void*)this))
				{
					m_input = buffer;
					execute_command(m_input);
					reclaim_focus = true;
				}
				ImGui::PopItemWidth();

				ImGui::SetItemDefaultFocus();
				if (reclaim_focus) ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
			}
		}

		ImGui::End();
	}

	void Console::execute_command(const std::string& p_command)
	{
		add_log(std::format("# {}\n", p_command));
		m_history.push_back(p_command);

		// Process command
		if (p_command == "CLEAR")
		{
			clear_log();
		}
		else if (p_command == "HELP")
		{
			std::string help_str = "TAB key - completion\nUp/Down keys - command history\nAvailable Commands:";
			for (const auto& command : m_commands)
				help_str += "\n- " + command;
			add_log(help_str);
		}
		else if (p_command == "HISTORY")
		{
			std::string historyStr = "Command history:\n";
			for (const auto& command : m_history)
				historyStr += "\n- " + command;
			add_log(historyStr);
		}
		else
			add_log({std::format("Unknown command: '{}'\n", p_command), Platform::Core::s_theme.error_text});

		// On command input, we scroll to bottom even if m_auto_scroll==false
		m_scroll_to_bottom = true;
	}

	static bool starts_with_case_insensitive(const std::string& p_str, const std::string& p_prefix)
	{
		if (p_str.length() < p_prefix.length()) return false;
		return std::equal(p_prefix.begin(), p_prefix.end(), p_str.begin(), [](char a, char b) { return std::tolower(a) == std::tolower(b); });
	}

	int Console::text_edit_callback(ImGuiInputTextCallbackData* p_data)
	{
		switch (p_data->EventFlag)
		{
			case ImGuiInputTextFlags_CallbackCompletion:
			{
				if (p_data->BufTextLen <= 0)
					return 0;

				// Build a list of candidates
				std::vector<std::string> candidates;
				for (const auto& command : m_commands)
				{
					if (m_input == command || starts_with_case_insensitive(command, m_input))
						candidates.push_back(command);
				}

				if (candidates.size() == 0)
					add_log(std::format("No match for '{}'", m_input));
				else if (candidates.size() == 1)
				{
					p_data->DeleteChars(0, p_data->CursorPos);
					p_data->InsertChars(p_data->CursorPos, candidates.front().c_str());
					m_input = p_data->Buf; // Update to match the completion
				}
				else
				{
					// List matches
					std::string candidatesStr = "Multiple matches:";
					for (const auto& candidate : candidates)
						candidatesStr += std::format("\n- {}", candidate);
					add_log(candidatesStr);
				}
				break;
			}
			case ImGuiInputTextFlags_CallbackHistory:
			{
				switch (p_data->EventKey)
				{
					case ImGuiKey_UpArrow:
						break;
					case ImGuiKey_DownArrow:
						break;
					default: break;
				}
			}
		}
		return 0;
	}
}