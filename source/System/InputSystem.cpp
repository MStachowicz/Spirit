#include "InputSystem.hpp"

// ECS
#include "Storage.hpp"

// System
#include "SceneSystem.hpp"

// Component
#include "Camera.hpp"

// Utility
#include "Logger.hpp"

// External
#include "GLFWInput.hpp"
#include "imgui.h"

namespace System
{
    InputSystem::InputSystem(System::SceneSystem& pSceneSystem)
        : mInputHandler(new GLFWInput
            ( std::bind(&InputSystem::onInput, this, std::placeholders::_1)
            , std::bind(&InputSystem::onMousePress, this, std::placeholders::_1, std::placeholders::_2)
            , std::bind(&InputSystem::onMouseMove, this, std::placeholders::_1, std::placeholders::_2)))
            , mSceneSystem(pSceneSystem)
    {}

    void InputSystem::pollEvents()
    {
        mInputHandler->pollEvents();
        mCapturedThisFrame = false;
    }

    void InputSystem::onMouseMove(const float& pXOffset, const float& pYOffset)
    {
        if (mCapturingMouse && !mCapturedThisFrame) // X,Y offsets can be large on the same frame mouse is captured. Skip on mCapturedThisFrame.
        {
            auto primaryCamera = mSceneSystem.getPrimaryCamera();
            if (primaryCamera)
                primaryCamera->ProcessMouseMove(pXOffset, pYOffset);
        }

    }

    void InputSystem::onMousePress(const InputAPI::MouseButton& pMouseButton, const InputAPI::Action& pAction)
    {
        if (!ImGui::GetIO().WantCaptureMouse)
        {
            switch (pMouseButton)
            {
            case InputAPI::MouseButton::MOUSE_LEFT:
            case InputAPI::MouseButton::MOUSE_MIDDLE:
                break;
            case InputAPI::MouseButton::MOUSE_RIGHT:
                if (pAction == InputAPI::Action::PRESS)
                {
                    if (!mCapturingMouse)
                    {
                        LOG_INFO("Captured mouse");
                        mInputHandler->setCursorMode(InputAPI::CursorMode::CAPTURED);
                        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
                        mCapturingMouse = true;
                        mCapturedThisFrame = true;
                    }
                    else
                    {
                        LOG_INFO("Mouse free");
                        mInputHandler->setCursorMode(InputAPI::CursorMode::NORMAL);
                        ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
                        mCapturingMouse = false;
                    }
                }
                break;
            default:
                LOG_WARN("Unknown mouse press {}", pMouseButton);
                break;
            }
        }
    }

    void InputSystem::onInput(const InputAPI::Key& pKeyPressed)
    {
        switch (pKeyPressed)
        {
            case InputAPI::Key::KEY_W:
            {
                if (auto* primaryCamera = mSceneSystem.getPrimaryCamera())
                    primaryCamera->move(Component::Camera::Forward);
                break;
            }
            case InputAPI::Key::KEY_S:
            {
                if (auto* primaryCamera = mSceneSystem.getPrimaryCamera())
                    primaryCamera->move(Component::Camera::Backward);
                break;
            }
            case InputAPI::Key::KEY_A:
            {
                if (auto* primaryCamera = mSceneSystem.getPrimaryCamera())
                    primaryCamera->move(Component::Camera::Left);
                break;
            }
            case InputAPI::Key::KEY_D:
            {
                if (auto* primaryCamera = mSceneSystem.getPrimaryCamera())
                    primaryCamera->move(Component::Camera::Right);
                break;
            }
            case InputAPI::Key::KEY_E:
            {
                if (auto* primaryCamera = mSceneSystem.getPrimaryCamera())
                    primaryCamera->move(Component::Camera::Up);
                break;
            }
            case InputAPI::Key::KEY_Q:
            {
                if (auto* primaryCamera = mSceneSystem.getPrimaryCamera())
                    primaryCamera->move(Component::Camera::Down);
                break;
            }
            case InputAPI::Key::KEY_ESCAPE:
                mCloseRequested = true;
                break;
            case InputAPI::Key::KEY_ENTER:
                break;
            case InputAPI::Key::KEY_UNKNOWN:
            default:
                LOG_WARN("Unknown key press of value");
                break;
        }
    }

    bool InputSystem::closeRequested()
    {
        return mInputHandler->closeRequested() || mCloseRequested;
    }
}