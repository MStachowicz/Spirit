#include "InputSystem.hpp"

// ECS
#include "Storage.hpp"

// System
#include "SceneSystem.hpp"

// Component
#include "Camera.hpp"

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
            if (auto* primaryCamera = mSceneSystem.getPrimaryCamera())
            {
                if (Platform::Core::is_key_down(Platform::Key::W)) primaryCamera->move(Component::Camera::Forward);
                if (Platform::Core::is_key_down(Platform::Key::S)) primaryCamera->move(Component::Camera::Backward);
                if (Platform::Core::is_key_down(Platform::Key::A)) primaryCamera->move(Component::Camera::Left);
                if (Platform::Core::is_key_down(Platform::Key::D)) primaryCamera->move(Component::Camera::Right);
                if (Platform::Core::is_key_down(Platform::Key::E)) primaryCamera->move(Component::Camera::Up);
                if (Platform::Core::is_key_down(Platform::Key::Q)) primaryCamera->move(Component::Camera::Down);
            }
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

    void InputSystem::onMouseMoved(const float& pXOffset, const float& pYOffset)
    {
        if (Platform::Core::getWindow().capturingMouse())
        {
            if (auto* primaryCamera = mSceneSystem.getPrimaryCamera())
                primaryCamera->ProcessMouseMove(pXOffset, pYOffset);
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