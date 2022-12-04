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
        Platform::Core::mKeyPressEvent.subscribe(std::bind(&InputSystem::onKeyPressed, this, std::placeholders::_1));
        Platform::Core::mMouseButtonEvent.subscribe(std::bind(&InputSystem::onMousePressed, this, std::placeholders::_1, std::placeholders::_2));
        Platform::Core::mMouseMoveEvent.subscribe(std::bind(&InputSystem::onMouseMoved, this, std::placeholders::_1, std::placeholders::_2));
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
        if (!Platform::Core::UICapturingMouse())
        {
            switch (pMouseButton)
            {
                case Platform::MouseButton::MOUSE_LEFT:
                case Platform::MouseButton::MOUSE_MIDDLE:
                    break;
                case Platform::MouseButton::MOUSE_RIGHT:
                    if (pAction == Platform::Action::PRESS)
                        Platform::Core::getWindow().setInputMode(Platform::Core::getWindow().capturingMouse() ? Platform::CursorMode::NORMAL : Platform::CursorMode::CAPTURED);
                    break;
                default:
                    LOG_WARN("Unknown mouse press {}", pMouseButton);
                    break;
            }
        }
    }

    void InputSystem::onKeyPressed(const Platform::Key& pKeyPressed)
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
            case Platform::Key::KEY_ESCAPE:
                Platform::Core::getWindow().requestClose();
                break;
            case Platform::Key::KEY_ENTER:
                break;
            case Platform::Key::KEY_UNKNOWN:
            default:
                LOG_WARN("Unknown key press of value");
                break;
        }
    }
}