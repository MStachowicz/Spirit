#include "InputSystem.hpp"
#include "SceneSystem.hpp"

#include "Component/FirstPersonCamera.hpp"
#include "Component/Input.hpp"
#include "Component/Transform.hpp"
#include "ECS/Storage.hpp"
#include "Platform/Core.hpp"
#include "Platform/Input.hpp"
#include "Platform/Window.hpp"

#include "Utility/Logger.hpp"
#include "Utility/Performance.hpp"

namespace System
{
	InputSystem::InputSystem(Platform::Input& p_input, Platform::Window& p_window, System::SceneSystem& p_scene_system)
		: m_update_count{0}
		, m_input{p_input}
		, m_window{p_window}
		, m_scene_system{p_scene_system}
	{}

	void InputSystem::update(const DeltaTime& p_delta_time)
	{
		PERF(InputSystemUpdate);

		m_update_count++;

		if (!m_input.keyboard_captured_by_UI())
		{
			m_scene_system.get_current_scene_entities().foreach([&](ECS::Entity& p_entity, Component::Input& p_input)
			{
				p_input.m_function(p_delta_time, p_entity, m_scene_system.get_current_scene_entities(), m_input);
			});
		}
	}
}