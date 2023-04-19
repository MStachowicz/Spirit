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
#include "InputDefinitions.hpp"
#include "Core.hpp"

namespace System
{
    InputSystem::InputSystem(System::SceneSystem& pSceneSystem)
        : mSceneSystem(pSceneSystem)
    {
        Platform::Core::mKeyPressEvent.subscribe(this, &InputSystem::onKeyPressed);
        Platform::Core::mMouseButtonEvent.subscribe(this, &InputSystem::onMousePressed);
        Platform::Core::mMouseMoveEvent.subscribe(this, &InputSystem::onMouseMoved);
    }

    void InputSystem::update()
    {
        if (!Platform::Core::UICapturingKeyboard() && Platform::Core::getWindow().capturingMouse())
        {
            mSceneSystem.getCurrentScene().foreach([](Component::Camera& p_camera)
            {
                if (p_camera.m_primary_camera)
                {
                    if (Platform::Core::is_key_down(Platform::Key::W)) p_camera.move(Component::Camera::move_direction::Forward);
                    if (Platform::Core::is_key_down(Platform::Key::S)) p_camera.move(Component::Camera::move_direction::Backward);
                    if (Platform::Core::is_key_down(Platform::Key::A)) p_camera.move(Component::Camera::move_direction::Left);
                    if (Platform::Core::is_key_down(Platform::Key::D)) p_camera.move(Component::Camera::move_direction::Right);
                    if (Platform::Core::is_key_down(Platform::Key::E)) p_camera.move(Component::Camera::move_direction::Up);
                    if (Platform::Core::is_key_down(Platform::Key::Q)) p_camera.move(Component::Camera::move_direction::Down);
                }
            });

        }
    }
    // use onKeyPressed to perform one-time actions. e.g. UI events are best not repeated every frame.
    // On the other hand, game logic is best suited to Platform::Core::is_key_down since this alows repeated events.
    void InputSystem::onKeyPressed(const Platform::Key& pKeyPressed)
    {
        switch (pKeyPressed)
        {
            case Platform::Key::Escape: Platform::Core::getWindow().requestClose();      break;
            case Platform::Key::F11:    Platform::Core::getWindow().toggle_fullscreen(); break;
            default: break;
        }
    }

    void InputSystem::onMouseMoved(const float& p_x_offset, const float& p_y_offset)
    {
        if (Platform::Core::getWindow().capturingMouse())
        {
            mSceneSystem.getCurrentScene().foreach([&p_x_offset, &p_y_offset](Component::Camera& p_camera)
            {
                if (p_camera.m_primary_camera)
                    p_camera.look(p_x_offset, p_y_offset);
            });
        }
    }

    void InputSystem::onMousePressed(const Platform::MouseButton& pMouseButton, const Platform::Action& pAction)
    {
        // InputSystem only reacts to inputs if the UI is not hovered and the mouse is captured by the window.
        if (!Platform::Core::UICapturingMouse() && Platform::Core::getWindow().capturingMouse())
        {
            switch (pMouseButton)
            {
                case Platform::MouseButton::MOUSE_LEFT:   break;
                case Platform::MouseButton::MOUSE_MIDDLE: break;
                case Platform::MouseButton::MOUSE_RIGHT:
                {
                    if (pAction == Platform::Action::PRESS)
                        Platform::Core::getWindow().setInputMode(Platform::CursorMode::NORMAL);
                    break;
                }
                default: break;
            }
        }
    }
}