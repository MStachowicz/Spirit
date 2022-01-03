#include "Input.hpp"
#include "Logger.hpp"
#include "GLFWInput.hpp"
#include "Camera.hpp"

Input::Input(Camera& pCamera)
: mCurrentCamera(pCamera)
{
    mInputHandler = new GLFWInput();
    mInputHandler->subscribeKeyCallback(std::bind(&Input::onInput, this, std::placeholders::_1));
}

void Input::pollEvents()
{
    mInputHandler->pollEvents();
}

void Input::onInput(const InputAPI::Key& pKeyPressed)
{
    switch (pKeyPressed)
    {
    case InputAPI::Key::KEY_W:
        mCurrentCamera.ProcessKeyboard(FORWARD);
        break;
    case InputAPI::Key::KEY_S:
         mCurrentCamera.ProcessKeyboard(BACKWARD);
        break;
    case InputAPI::Key::KEY_A:
         mCurrentCamera.ProcessKeyboard(LEFT);
        break;
    case InputAPI::Key::KEY_D:
         mCurrentCamera.ProcessKeyboard(RIGHT);
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