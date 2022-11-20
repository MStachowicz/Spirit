#include "Input.hpp"

#include "Storage.hpp"

#include "Camera.hpp"

#include "Logger.hpp"

#include "GLFWInput.hpp"
#include "imgui.h"

Input::Input(ECS::Storage& pStorage)
    : mInputHandler(new GLFWInput
        ( std::bind(&Input::onInput, this, std::placeholders::_1)
        , std::bind(&Input::onMousePress, this, std::placeholders::_1, std::placeholders::_2)
        , std::bind(&Input::onMouseMove, this, std::placeholders::_1, std::placeholders::_2)))
        , mStorage(pStorage)
{}

void Input::pollEvents()
{
    mInputHandler->pollEvents();
    mCapturedThisFrame = false;
}

void Input::onMouseMove(const float& pXOffset, const float& pYOffset)
{
    if (mCapturingMouse && !mCapturedThisFrame) // X,Y offsets can be large on the same frame mouse is captured. Skip on mCapturedThisFrame.
    {
        auto primaryCamera = getPrimaryCamera(mStorage);
        if (primaryCamera)
            primaryCamera->ProcessMouseMove(pXOffset, pYOffset);
    }

}

void Input::onMousePress(const InputAPI::MouseButton& pMouseButton, const InputAPI::Action& pAction)
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

void Input::onInput(const InputAPI::Key& pKeyPressed)
{
    switch (pKeyPressed)
    {
        case InputAPI::Key::KEY_W:
        {
            auto primaryCamera = getPrimaryCamera(mStorage);
            if (primaryCamera)
                primaryCamera->move(Component::Camera::Forward);
            break;
        }
        case InputAPI::Key::KEY_S:
        {
            auto primaryCamera = getPrimaryCamera(mStorage);
            if (primaryCamera)
                primaryCamera->move(Component::Camera::Backward);
            break;
        }
        case InputAPI::Key::KEY_A:
        {
            auto primaryCamera = getPrimaryCamera(mStorage);
            if (primaryCamera)
                primaryCamera->move(Component::Camera::Left);
            break;
        }
        case InputAPI::Key::KEY_D:
        {
            auto primaryCamera = getPrimaryCamera(mStorage);
            if (primaryCamera)
                primaryCamera->move(Component::Camera::Right);
            break;
        }
        case InputAPI::Key::KEY_E:
        {
            auto primaryCamera = getPrimaryCamera(mStorage);
            if (primaryCamera)
                primaryCamera->move(Component::Camera::Up);
            break;
        }
        case InputAPI::Key::KEY_Q:
        {
            auto primaryCamera = getPrimaryCamera(mStorage);
            if (primaryCamera)
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

bool Input::closeRequested()
{
    return mInputHandler->closeRequested() || mCloseRequested;
}

Component::Camera* Input::getPrimaryCamera(ECS::Storage& pStorage)
{
    Component::Camera* primaryCamera;
    pStorage.foreach([&primaryCamera](Component::Camera& pCamera)
    {
        if (pCamera.mPrimaryCamera)
            primaryCamera = &pCamera;
    });

    return primaryCamera;
}