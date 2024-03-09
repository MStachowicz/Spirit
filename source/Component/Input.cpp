#include "Component/Input.hpp"
#include "Component/FirstPersonCamera.hpp"
#include "Component/RigidBody.hpp"
#include "Component/Transform.hpp"
#include "ECS/Storage.hpp"

#include "Platform/Input.hpp"
#include "Utility/Utility.hpp"

namespace Component
{
	InputFunction Input::Camera_Move_Look = [](const DeltaTime& p_delta_time, const ECS::Entity& p_entity, ECS::Storage& p_storage, Platform::Input& p_input)
	{
		if (p_storage.has_components<Component::FirstPersonCamera>(p_entity))
		{
			auto& p_camera = p_storage.get_component<Component::FirstPersonCamera>(p_entity);
			if (p_camera.m_primary)
			{
				const auto mouse_offset = p_input.cursor_delta();
				if (mouse_offset.x != 0.f || mouse_offset.y != 0.f)
					p_camera.mouse_look(mouse_offset);

				Component::RigidBody* body = nullptr;
				if (p_storage.has_components<Component::RigidBody>(p_entity))
					body = &p_storage.get_component<Component::RigidBody>(p_entity);

				Component::Transform* transform = nullptr;
				if (p_storage.has_components<Component::Transform>(p_entity))
					transform = &p_storage.get_component<Component::Transform>(p_entity);

				if (transform || body)
				{
					if (p_input.is_key_down(Platform::Key::W)) p_camera.move(p_delta_time, Component::Transform::MoveDirection::Forward, transform, body);
					if (p_input.is_key_down(Platform::Key::S)) p_camera.move(p_delta_time, Component::Transform::MoveDirection::Backward, transform, body);
					if (p_input.is_key_down(Platform::Key::D)) p_camera.move(p_delta_time, Component::Transform::MoveDirection::Right, transform, body);
					if (p_input.is_key_down(Platform::Key::A)) p_camera.move(p_delta_time, Component::Transform::MoveDirection::Left, transform, body);
					if (p_input.is_key_down(Platform::Key::Q)) p_camera.move(p_delta_time, Component::Transform::MoveDirection::Down, transform, body);
					if (p_input.is_key_down(Platform::Key::E)) p_camera.move(p_delta_time, Component::Transform::MoveDirection::Up, transform, body);
				}
			}
		}
	};
}