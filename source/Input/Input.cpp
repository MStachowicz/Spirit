#include "Input.hpp"
#include "Logger.hpp"
#include "GLFWInput.hpp"
#include "Camera.hpp"

#include "imgui.h"

Input::Input(Camera& pCamera)
: mCurrentCamera(pCamera)
, mInputHandler(
    new GLFWInput(
          std::bind(&Input::onInput, this, std::placeholders::_1)
        , std::bind(&Input::onMousePress, this, std::placeholders::_1, std::placeholders::_2)
        , std::bind(&Input::onMouseMove, this, std::placeholders::_1, std::placeholders::_2)))
{}

void Input::pollEvents()
{
    mInputHandler->pollEvents();
}

void Input::onMouseMove(const float& pXOffset, const float& pYOffset)
{
    if (mCapturingMouse)
        mCurrentCamera.ProcessMouseMove(pXOffset, pYOffset);
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
                    mCapturingMouse = true;
                }
                else
                {
                    LOG_INFO("Mouse free");
                    mInputHandler->setCursorMode(InputAPI::CursorMode::NORMAL);
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
        mCurrentCamera.move(Camera::Forward);
        break;
    case InputAPI::Key::KEY_S:
         mCurrentCamera.move(Camera::Backward);
        break;
    case InputAPI::Key::KEY_A:
         mCurrentCamera.move(Camera::Left);
        break;
    case InputAPI::Key::KEY_D:
         mCurrentCamera.move(Camera::Right);
        break;
    case InputAPI::Key::KEY_E:
         mCurrentCamera.move(Camera::Up);
        break;
    case InputAPI::Key::KEY_Q:
         mCurrentCamera.move(Camera::Down);
        break;
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