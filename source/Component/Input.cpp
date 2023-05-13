#include "Component/Input.hpp"
#include "Component/Camera.hpp"

#include "ECS/Storage.hpp"

#include "Platform/Input.hpp"

namespace Component
{
    InputFunction Input::Move = [](const ECS::Entity& p_entity, ECS::Storage& p_storage, Platform::Input& p_input)
    {
        if (p_storage.hasComponents<Component::Camera>(p_entity))
        {
            auto& p_camera = p_storage.getComponentMutable<Component::Camera>(p_entity);

            if (p_camera.m_primary_camera)
            {
                if (p_input.is_key_down(Platform::Key::W)) p_camera.move(Component::Camera::move_direction::Forward);
                if (p_input.is_key_down(Platform::Key::S)) p_camera.move(Component::Camera::move_direction::Backward);
                if (p_input.is_key_down(Platform::Key::A)) p_camera.move(Component::Camera::move_direction::Left);
                if (p_input.is_key_down(Platform::Key::D)) p_camera.move(Component::Camera::move_direction::Right);
                if (p_input.is_key_down(Platform::Key::E)) p_camera.move(Component::Camera::move_direction::Up);
                if (p_input.is_key_down(Platform::Key::Q)) p_camera.move(Component::Camera::move_direction::Down);
            }
        }
    };
}