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
                case Platform::MouseButton::MOUSE_LEFT:
                    break;
                case Platform::MouseButton::MOUSE_MIDDLE:
                    break;
                case Platform::MouseButton::MOUSE_RIGHT:
                {
                    if (pAction == Platform::Action::PRESS)
                        Platform::Core::getWindow().setInputMode(Platform::CursorMode::NORMAL);
                    break;
                }
                    break;
                default:
                    LOG_WARN("Unknown mouse press");
                    break;
            }
        }
    }

    void InputSystem::onKeyPressed(const Platform::Key& pKeyPressed)
    {
        if (pKeyPressed == Platform::Key::KEY_ESCAPE) // Regardless of state, close on escape
            Platform::Core::getWindow().requestClose();

        if (!Platform::Core::UICapturingKeyboard() && Platform::Core::getWindow().capturingMouse())
        {
            switch (pKeyPressed)
            {
                case Platform::Key::KEY_W:
                {
                    if (auto* primaryCamera = mSceneSystem.getPrimaryCamera())
                        primaryCamera->move(Component::Camera::Forward);
                    break;
                }
                case Platform::Key::KEY_S:
                {
                    if (auto* primaryCamera = mSceneSystem.getPrimaryCamera())
                        primaryCamera->move(Component::Camera::Backward);
                    break;
                }
                case Platform::Key::KEY_A:
                {
                    if (auto* primaryCamera = mSceneSystem.getPrimaryCamera())
                        primaryCamera->move(Component::Camera::Left);
                    break;
                }
                case Platform::Key::KEY_D:
                {
                    if (auto* primaryCamera = mSceneSystem.getPrimaryCamera())
                        primaryCamera->move(Component::Camera::Right);
                    break;
                }
                case Platform::Key::KEY_E:
                {
                    if (auto* primaryCamera = mSceneSystem.getPrimaryCamera())
                        primaryCamera->move(Component::Camera::Up);
                    break;
                }
                case Platform::Key::KEY_Q:
                {
                    if (auto* primaryCamera = mSceneSystem.getPrimaryCamera())
                        primaryCamera->move(Component::Camera::Down);
                    break;
                }
                case Platform::Key::KEY_ENTER:
                case Platform::Key::KEY_UNKNOWN:
                default:
                    break;
            }
        }
    }
}