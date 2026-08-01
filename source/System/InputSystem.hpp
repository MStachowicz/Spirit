#pragma once

#include "Utility/Config.hpp"

#include <stdint.h>

namespace Platform
{
	enum class Key : uint8_t;
	enum class MouseButton : uint8_t;
	enum class Action : uint8_t;
	enum class CursorMode : uint8_t;
	class Input;
}
namespace System
{
	class SceneSystem;
	class IPhysicsSystem;

	// Listens to platform input events.
	class InputSystem
	{
	public:
		InputSystem(Platform::Input& p_input, SceneSystem& p_scene_system);
		void update(const DeltaTime& p_delta_time);

		size_t m_update_count;
	private:
		Platform::Input& m_input;
		SceneSystem& m_scene_system;
	};
}