#pragma once

#include "Utility/EventDispatcher.hpp"

#include "glm/vec2.hpp"

#include <array>

typedef struct GLFWwindow GLFWwindow;

namespace Platform
{
	// Supports 256 Keys
	enum class Key : uint8_t
	{
		Num_0, Num_1, Num_2, Num_3, Num_4, Num_5, Num_6, Num_7, Num_8, Num_9,
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
		Space, Escape, Enter, Tab,
		Left_Arrow, Right_Arrow, Up_Arrow, Down_Arrow,
		Unknown
	};
	enum class MouseButton : uint8_t
	{
		Left, Middle, Right,
		Button_1, Button_2, Button_3, Button_4, Button_5,
		Unknown
	};
	enum class Action : uint8_t
	{
		Press, Release, Repeat,
		Unknown
	};

	enum class CursorMode : uint8_t
	{
		Normal,   // Cursor is visible and not being captured by the window.
		Hidden,   // Cursor is hidden when hovering over the window and not being captured by it.
		Captured // Cursor is hidden and captured by the window.
	};

	// Maintains the state of the UI at the current frame/update cycle.
	class Input
	{
		constexpr static auto Max_Key_Index = std::numeric_limits<std::underlying_type_t<Key>>::max();
		constexpr static auto Max_Mouse_Index = std::numeric_limits<std::underlying_type_t<MouseButton>>::max();

		std::array<bool, Max_Key_Index> m_keys_pressed;
		std::array<bool, Max_Mouse_Index> m_mouse_buttons_pressed;
		glm::vec2 m_cursor_position; // The cursor position relative to the upper left corner of the window.
		glm::vec2 m_cursor_delta;    // The pixels the mouse cursor moved by since the last Input::update call.
		CursorMode m_cursor_mode;
		bool m_captured_this_frame;

		// The remaining functions allow Platform::Window to callback into Input.
		friend class Window;
		GLFWwindow* m_handle; // This is a read only handle to the window this Input object depends on. Required to set cursor modes.
		void glfw_key_press(int p_key, int p_scancode, int p_action, int p_mode);
		void glfw_mouse_press(int p_button, int p_action, int p_modifiers);
		void glfw_mouse_move(double p_cursor_new_x_pos, double p_cursor_new_y_pos);
		void glfw_mouse_scroll(double p_x_offset, double p_y_offset);

		static constexpr Key glfw_to_key(int p_glfw_key);
		static constexpr MouseButton glfw_to_mouse_button(int p_glfw_mouse_button);
		static constexpr Action glfw_to_action(int p_glfw_action);

	public:
		Utility::EventDispatcher<Key, Action> m_key_event;
		Utility::EventDispatcher<MouseButton, Action> m_mouse_button_event;
		Utility::EventDispatcher<glm::vec2> m_mouse_move_event;
		Utility::EventDispatcher<glm::vec2> m_mouse_scroll_event;

		Input() noexcept;
		// Polls for events and updates the state.
		// Cause the window and input callbacks associated with those events to be called.
		void update();

		bool is_key_down(Key p_key) const;
		bool is_mouse_down(MouseButton p_button) const;
		// Returns the most recent cursor position change (change since the last Input update call).
		glm::vec2 cursor_delta() const;
		// Returns the cursor position relative to the upper left corner of the window.
		glm::vec2 cursor_position() const;
		// Set the cursor style for the window associated with this Input.
		void set_cursor_mode(CursorMode p_cursor_mode);
		CursorMode cursor_mode() const { return m_cursor_mode; };
		bool cursor_captured() const { return m_cursor_mode == CursorMode::Captured && !m_captured_this_frame; }

		// Is the mouse hovering over any UI elements. Click events during this state will generally be absorbed by the UI.
		bool cursor_over_UI() const;
		// Is the UI capturing keyboard input, such when a user clicks on an input box.
		bool keyboard_captured_by_UI() const;
	};
}