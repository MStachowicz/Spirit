#include "Component/Input.hpp"
#include "Component/FirstPersonCamera.hpp"
#include "Component/Collider.hpp"
#include "Component/Transform.hpp"
#include "ECS/Storage.hpp"

#include "Platform/Input.hpp"
#include "Utility/Utility.hpp"

namespace Component
{
	InputFunction Input::Camera_Move_Look = [](const DeltaTime& p_delta_time, const ECS::Entity& p_entity, ECS::Storage& p_storage, Platform::Input& p_input)
	{
		if (p_storage.has_components<Component::FirstPersonCamera, Component::Transform>(p_entity))
		{
			auto& p_camera  = p_storage.get_component<Component::FirstPersonCamera>(p_entity);

			if (p_camera.m_primary)
			{
				auto& transform = p_storage.get_component<Component::Transform>(p_entity);
				auto forward    = transform.forward();
				auto right      = transform.right();
				auto up         = transform.up();
				auto speed      = p_camera.m_move_speed * p_delta_time.count();

				if (p_input.is_modifier_down(Platform::Modifier::Shift))
					speed *= 3.f;

				const auto mouse_offset = p_input.cursor_delta();
				if (mouse_offset.x != 0.f || mouse_offset.y != 0.f)
					p_camera.mouse_look(mouse_offset);

				if (p_storage.has_components<Component::Collider>(p_entity))
				{
					glm::vec3 force = glm::vec3(0.f);
					if (p_input.is_key_down(Platform::Key::W)) force += forward  * speed;
					if (p_input.is_key_down(Platform::Key::S)) force += -forward * speed;
					if (p_input.is_key_down(Platform::Key::D)) force += right    * speed;
					if (p_input.is_key_down(Platform::Key::A)) force += -right   * speed;
					if (p_input.is_key_down(Platform::Key::Q)) force += up       * speed;
					if (p_input.is_key_down(Platform::Key::E)) force += -up      * speed;

					Component::Collider* collider = &p_storage.get_component<Component::Collider>(p_entity);
					collider->apply_force(force);
				}
				else
				{
					if (p_input.is_key_down(Platform::Key::W)) transform.m_position += forward * speed;
					if (p_input.is_key_down(Platform::Key::S)) transform.m_position -= forward * speed;
					if (p_input.is_key_down(Platform::Key::D)) transform.m_position += right   * speed;
					if (p_input.is_key_down(Platform::Key::A)) transform.m_position -= right   * speed;
					if (p_input.is_key_down(Platform::Key::Q)) transform.m_position += up      * speed;
					if (p_input.is_key_down(Platform::Key::E)) transform.m_position -= up      * speed;
				}
			}
		}
	};
}