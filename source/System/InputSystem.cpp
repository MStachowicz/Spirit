#include "InputSystem.hpp"

// ECS
#include "Storage.hpp"

// System
#include "SceneSystem.hpp"

// Component
#include "Camera.hpp"
#include "Transform.hpp"

// Utility
#include "Logger.hpp"

// Platform
#include "Input.hpp"
#include "Core.hpp"
#include "Window.hpp"

namespace System
{
    InputSystem::InputSystem(Platform::Input& p_input, Platform::Window& p_window, System::SceneSystem& pSceneSystem)
        : m_input{p_input}
        , m_window{p_window}
        , mSceneSystem{pSceneSystem}
    {
        m_input.m_key_event.subscribe(this, &InputSystem::on_key_event);
    }

    void InputSystem::update()
    {
        if (!m_input.cursor_captured())
            return;

        if (!m_input.keyboard_captured_by_UI())
        {
            mSceneSystem.getCurrentScene().foreach([this](Component::Camera& p_camera)
            {
                if (p_camera.m_primary_camera)
                {
                    if (m_input.is_key_down(Platform::Key::W)) p_camera.move(Component::Camera::move_direction::Forward);
                    if (m_input.is_key_down(Platform::Key::S)) p_camera.move(Component::Camera::move_direction::Backward);
                    if (m_input.is_key_down(Platform::Key::A)) p_camera.move(Component::Camera::move_direction::Left);
                    if (m_input.is_key_down(Platform::Key::D)) p_camera.move(Component::Camera::move_direction::Right);
                    if (m_input.is_key_down(Platform::Key::E)) p_camera.move(Component::Camera::move_direction::Up);
                    if (m_input.is_key_down(Platform::Key::Q)) p_camera.move(Component::Camera::move_direction::Down);
                }
            });
        }

        mSceneSystem.getCurrentScene().foreach([this](Component::Camera& p_camera)
        {
            if (p_camera.m_primary_camera)
                p_camera.look(m_input.cursor_delta());
        });
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