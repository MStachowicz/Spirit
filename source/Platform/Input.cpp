#include "Input.hpp"

#include "Utility/Logger.hpp"

#include "GLFW/glfw3.h"
#include "imgui.h"

namespace Platform
{
	Input::Input() noexcept
		: m_keys_pressed{false}
		, m_cursor_position{0.f, 0.f} // Initial value set in Window constructor.
		, m_cursor_delta{0.f, 0.f}
		, m_cursor_mode{CursorMode::Normal}
		, m_captured_this_frame{false}
		, m_handle{nullptr} // Initial value set in Window constructor
		, m_key_event{}
		, m_mouse_event{}
	{}

	void Input::update()
	{
		m_captured_this_frame = false;
		m_cursor_delta = {0.f, 0.f};
		glfwPollEvents();
	}

	bool Input::is_key_down(const Key& p_key) const
	{
		return m_keys_pressed[static_cast<std::underlying_type_t<Key>>(p_key)];
	}
	glm::vec2 Input::cursor_delta() const
	{
		return m_cursor_delta;
	}
	glm::vec2 Input::cursor_position() const
	{
		return m_cursor_position;
	}

	void Input::set_cursor_mode(const CursorMode& p_cursor_mode)
	{
		m_cursor_mode = p_cursor_mode;
		switch (m_cursor_mode)
		{
			case CursorMode::Normal:
				glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				LOG("[INPUT] Cursor mode set to normal");
				break;
			case CursorMode::Hidden:
				glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
				LOG("[INPUT] Cursor mode set to hidden");
				break;
			case CursorMode::Captured:
				m_captured_this_frame = true;
				glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				LOG("[INPUT] Cursor mode set to captured");
				break;
			default:
				break;
		}
	}
	bool Input::cursor_over_UI() const
	{
		return ImGui::GetIO().WantCaptureMouse;
	}
	bool Input::keyboard_captured_by_UI() const
	{
		return ImGui::GetIO().WantCaptureKeyboard;
	}

	void Input::glfw_key_press(int p_key, int p_scancode, int p_action, int p_mode)
	{
		(void)p_scancode; (void)p_mode; // Unused parameters, required signature for GLFW callbacks, cant be deleted

		if (p_action == GLFW_PRESS)
		{
			m_keys_pressed[static_cast<std::underlying_type_t<Key>>(glfw_to_key(p_key))] = true;
			m_key_event.dispatch(glfw_to_key(p_key), glfw_to_action(p_action));
		}
		else if (p_action == GLFW_RELEASE)
		{
			m_keys_pressed[static_cast<std::underlying_type_t<Key>>(glfw_to_key(p_key))] = false;
		}
	}
	void Input::glfw_mouse_press(int p_button, int p_action, int p_modifiers)
	{ (void)p_modifiers; // Unused parameter, required signature for GLFW callbacks, cant be deleted
		m_mouse_event.dispatch(glfw_to_mouse_button(p_button), glfw_to_action(p_action));
	}
	void Input::glfw_mouse_move(double p_cursor_new_x_pos, double p_cursor_new_y_pos)
	{
		// p_cursor_new_x_pos and p_cursor_new_y_pos represent the position, in screen coordinates,
		// relative to the upper-left corner of the content area of the window.

		glm::vec2 old_cursor_pos = m_cursor_position;
		m_cursor_position = {static_cast<float>(p_cursor_new_x_pos), static_cast<float>(p_cursor_new_y_pos)};
		// reversed y-coordinates to stay relative to top-left
		m_cursor_delta = {m_cursor_position.x - old_cursor_pos.x, old_cursor_pos.y - m_cursor_position.y};
	}

	constexpr Key Input::glfw_to_key(int p_glfw_key)
	{
		switch (p_glfw_key)
		{
			case GLFW_KEY_0:      return Key::Num_0;
			case GLFW_KEY_1:      return Key::Num_1;
			case GLFW_KEY_2:      return Key::Num_2;
			case GLFW_KEY_3:      return Key::Num_3;
			case GLFW_KEY_4:      return Key::Num_4;
			case GLFW_KEY_5:      return Key::Num_5;
			case GLFW_KEY_6:      return Key::Num_6;
			case GLFW_KEY_7:      return Key::Num_7;
			case GLFW_KEY_8:      return Key::Num_8;
			case GLFW_KEY_9:      return Key::Num_9;
			case GLFW_KEY_A:      return Key::A;
			case GLFW_KEY_B:      return Key::B;
			case GLFW_KEY_C:      return Key::C;
			case GLFW_KEY_D:      return Key::D;
			case GLFW_KEY_E:      return Key::E;
			case GLFW_KEY_F:      return Key::F;
			case GLFW_KEY_G:      return Key::G;
			case GLFW_KEY_H:      return Key::H;
			case GLFW_KEY_I:      return Key::I;
			case GLFW_KEY_J:      return Key::J;
			case GLFW_KEY_K:      return Key::K;
			case GLFW_KEY_L:      return Key::L;
			case GLFW_KEY_M:      return Key::M;
			case GLFW_KEY_N:      return Key::N;
			case GLFW_KEY_O:      return Key::O;
			case GLFW_KEY_P:      return Key::P;
			case GLFW_KEY_Q:      return Key::Q;
			case GLFW_KEY_R:      return Key::R;
			case GLFW_KEY_S:      return Key::S;
			case GLFW_KEY_T:      return Key::T;
			case GLFW_KEY_U:      return Key::U;
			case GLFW_KEY_V:      return Key::V;
			case GLFW_KEY_W:      return Key::W;
			case GLFW_KEY_X:      return Key::X;
			case GLFW_KEY_Y:      return Key::Y;
			case GLFW_KEY_Z:      return Key::Z;
			case GLFW_KEY_F1:     return Key::F1;
			case GLFW_KEY_F2:     return Key::F2;
			case GLFW_KEY_F3:     return Key::F3;
			case GLFW_KEY_F4:     return Key::F4;
			case GLFW_KEY_F5:     return Key::F5;
			case GLFW_KEY_F6:     return Key::F6;
			case GLFW_KEY_F7:     return Key::F7;
			case GLFW_KEY_F8:     return Key::F8;
			case GLFW_KEY_F9:     return Key::F9;
			case GLFW_KEY_F10:    return Key::F10;
			case GLFW_KEY_F11:    return Key::F11;
			case GLFW_KEY_F12:    return Key::F12;
			case GLFW_KEY_SPACE:  return Key::Space;
			case GLFW_KEY_ESCAPE: return Key::Escape;
			case GLFW_KEY_ENTER:  return Key::Enter;
			case GLFW_KEY_TAB:    return Key::Tab;
			case GLFW_KEY_UNKNOWN:
			default:
				LOG_WARN("[INPUT] Could not convert GLFW key '{}' to Platform::Key", p_glfw_key);
				return Key::Unknown;
		}
	}
	constexpr MouseButton Input::glfw_to_mouse_button(int p_glfw_mouse_button)
	{
		switch (p_glfw_mouse_button)
		{
			case GLFW_MOUSE_BUTTON_LEFT:   return MouseButton::Left;
			case GLFW_MOUSE_BUTTON_MIDDLE: return MouseButton::Middle;
			case GLFW_MOUSE_BUTTON_RIGHT:  return MouseButton::Right;
			case GLFW_MOUSE_BUTTON_4:      return MouseButton::Button_1;
			case GLFW_MOUSE_BUTTON_5:      return MouseButton::Button_2;
			case GLFW_MOUSE_BUTTON_6:      return MouseButton::Button_3;
			case GLFW_MOUSE_BUTTON_7:      return MouseButton::Button_4;
			case GLFW_MOUSE_BUTTON_8:      return MouseButton::Button_5;
			default:
				LOG_WARN("[INPUT] Could not convert GLFW mouse button '{}' to MouseButton", p_glfw_mouse_button);
				return MouseButton::Unknown;
		}
	}
	constexpr Action Input::glfw_to_action(int p_glfw_action)
	{
		switch (p_glfw_action)
		{
			case GLFW_PRESS:   return Action::Press;
			case GLFW_RELEASE: return Action::Release;
			case GLFW_REPEAT:  return Action::Repeat;
			default:
				LOG_ERROR("[INPUT] Could not convert GLFW action '{}' to Action", p_glfw_action);
				return Action::Unknown;
		}
	}
}