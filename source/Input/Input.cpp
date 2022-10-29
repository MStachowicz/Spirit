#include "Input.hpp"

#include "Managers/CameraManager.hpp"

#include "Logger.hpp"

#include "GLFWInput.hpp"
#include "imgui.h"

Input::Input(Manager::CameraManager& pCameraManager)
    : mInputHandler(new GLFWInput
        ( std::bind(&Input::onInput, this, std::placeholders::_1)
        , std::bind(&Input::onMousePress, this, std::placeholders::_1, std::placeholders::_2)
        , std::bind(&Input::onMouseMove, this, std::placeholders::_1, std::placeholders::_2)))
    , mCameraManager(pCameraManager)
{}

void Input::pollEvents()
{
    mInputHandler->pollEvents();
}

void Input::onMouseMove(const float& pXOffset, const float& pYOffset)
{
    if (mCapturingMouse)
        mCameraManager.modifyPrimaryCamera([&pXOffset, &pYOffset](auto& pPrimaryCamera)
        {
            pPrimaryCamera.ProcessMouseMove(pXOffset, pYOffset);
        });
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
            mCameraManager.modifyPrimaryCamera([](auto& pPrimaryCamera) { pPrimaryCamera.move(Data::Camera::Forward); });
            break;
        case InputAPI::Key::KEY_S:
            mCameraManager.modifyPrimaryCamera([](auto& pPrimaryCamera) { pPrimaryCamera.move(Data::Camera::Backward); });
            break;
        case InputAPI::Key::KEY_A:
            mCameraManager.modifyPrimaryCamera([](auto& pPrimaryCamera) { pPrimaryCamera.move(Data::Camera::Left); });
            break;
        case InputAPI::Key::KEY_D:
            mCameraManager.modifyPrimaryCamera([](auto& pPrimaryCamera) { pPrimaryCamera.move(Data::Camera::Right); });
            break;
        case InputAPI::Key::KEY_E:
            mCameraManager.modifyPrimaryCamera([](auto& pPrimaryCamera) { pPrimaryCamera.move(Data::Camera::Up); });
            break;
        case InputAPI::Key::KEY_Q:
            mCameraManager.modifyPrimaryCamera([](auto& pPrimaryCamera) { pPrimaryCamera.move(Data::Camera::Down); });
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