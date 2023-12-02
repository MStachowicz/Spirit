#include "InputSystem.hpp"
#include "SceneSystem.hpp"

#include "Component/Camera.hpp"
#include "Component/Input.hpp"
#include "Component/Transform.hpp"
#include "ECS/Storage.hpp"
#include "Platform/Core.hpp"
#include "Platform/Input.hpp"
#include "Platform/Window.hpp"
#include "Utility/Logger.hpp"

namespace System
{
	InputSystem::InputSystem(Platform::Input& p_input, Platform::Window& p_window, System::SceneSystem& p_scene_system)
		: m_update_count{0}
		, m_input{p_input}
		, m_window{p_window}
		, m_scene_system{p_scene_system}
	{
		m_input.m_key_event.subscribe(this, &InputSystem::on_key_event);
	}

	void InputSystem::update(const DeltaTime& p_delta_time)
	{
		m_update_count++;

		if (!m_input.cursor_captured())
			return;

		if (!m_input.keyboard_captured_by_UI())
		{
			m_scene_system.get_current_scene().foreach([&](ECS::Entity& p_entity, Component::Input& p_input)
			{
				p_input.m_function(p_delta_time, p_entity, m_scene_system.get_current_scene(), m_input);
			});
		}
	}
	// use onKeyPressed to perform one-time actions. e.g. UI events are best not repeated every frame.
	// On the other hand, game logic is best suited to Platform::Core::is_key_down since this alows repeated events.
	void InputSystem::on_key_event(Platform::Key p_key, Platform::Action p_action)
	{
		if (p_action == Platform::Action::Press)
		{
			switch (p_key)
			{
				case Platform::Key::Escape: m_window.request_close();     break;
				case Platform::Key::F11:    m_window.toggle_fullscreen(); break;
				default: break;
			}
		}
	}
}