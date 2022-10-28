#include "GLFWInput.hpp"
#include "GLFW/glfw3.h"
#include "Logger.hpp"
#include "OpenGLWindow.hpp"

GLFWInput::GLFWInput(
    std::function<void(const InputAPI::Key&)> pOnKeyPressCallback
    , std::function<void(const InputAPI::MouseButton&, const InputAPI::Action& pAction)> pOnMousePressCallback
    , std::function<void(const float&, const float&)> pOnMouseMoveCallback)
: InputHandler(pOnKeyPressCallback, pOnMousePressCallback, pOnMouseMoveCallback)
, mCloseRequested(false)
{
    glfwSetInputMode(OpenGL::OpenGLWindow::getActiveWindowHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetKeyCallback(OpenGL::OpenGLWindow::getActiveWindowHandle(), keyCallback);
    glfwSetWindowCloseCallback(OpenGL::OpenGLWindow::getActiveWindowHandle(), windowCloseRequestCallback);
    glfwSetCursorPosCallback(OpenGL::OpenGLWindow::getActiveWindowHandle(), mouseMoveCallback);
    glfwSetMouseButtonCallback(OpenGL::OpenGLWindow::getActiveWindowHandle(), mouseButtonCallback);
    currentActiveInputHandler = this;
};

void GLFWInput::pollEvents()
{
    glfwPollEvents();
}

bool GLFWInput::closeRequested()
{
    return mCloseRequested;
}

void GLFWInput::setCursorMode(const InputAPI::CursorMode& pCursorMode)
{
    // GLFW_CURSOR_NORMAL: Regular arrow cursor, motion is not limited.
    // GLFW_CURSOR_HIDDEN: Cursor hidden when it's over the window, motion is not limited.
    // GLFW_CURSOR_DISABLED: Hides the cursor and locks it to window, motion is unlimited.
    // By default, the cursor mode is GLFW_CURSOR_NORMAL

    switch (pCursorMode)
    {
    case InputAPI::CursorMode::NORMAL:
        glfwSetInputMode(OpenGL::OpenGLWindow::getActiveWindowHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        break;
    case InputAPI::CursorMode::HIDDEN:
        glfwSetInputMode(OpenGL::OpenGLWindow::getActiveWindowHandle(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        break;
    case InputAPI::CursorMode::CAPTURED:
        glfwSetInputMode(OpenGL::OpenGLWindow::getActiveWindowHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        break;
    case InputAPI::CursorMode::UNKNOWN:
    default:
        LOG_ERROR("Could not convert cursor mode requested ({}) to GLFW cursor mode", pCursorMode);
        break;
    }
}

void GLFWInput::windowCloseRequestCallback(GLFWwindow* pWindow)
{
    currentActiveInputHandler->mCloseRequested = true;
}

void GLFWInput::keyCallback(GLFWwindow* pWindow, int pKey, int pScancode, int pAction, int pMode)
{
	if (pAction == GLFW_PRESS)
        currentActiveInputHandler->onKeyPress(convert(pKey));
}

void GLFWInput::mouseMoveCallback(GLFWwindow* pWindow, double pNewXPosition, double pNewYPosition)
{
    if (mLastXPosition == -1.0 || mLastYPosition == -1.0) // First mouse input
    {
        mLastXPosition = pNewXPosition;
        mLastYPosition = pNewYPosition;
    }

    const float xOffset = static_cast<float>(pNewXPosition - mLastXPosition);
    const float yOffset = static_cast<float>(mLastYPosition - pNewYPosition); // reversed since y-coordinates go from bottom to top

    mLastXPosition = pNewXPosition;
    mLastYPosition = pNewYPosition;

    currentActiveInputHandler->onMouseMove(xOffset, yOffset);
}

void GLFWInput::mouseButtonCallback(GLFWwindow* pWindow, int pButton, int pAction, int pModifiers)
{
    currentActiveInputHandler->onMousePress(convertMouseButton(pButton), convertAction(pAction));
}

InputAPI::Key GLFWInput::convert(const int& pKeyInput)
{
	switch (pKeyInput)
	{
       case GLFW_KEY_0: 		return InputAPI::Key::KEY_0;
       case GLFW_KEY_1: 		return InputAPI::Key::KEY_1;
       case GLFW_KEY_2: 		return InputAPI::Key::KEY_2;
       case GLFW_KEY_3: 		return InputAPI::Key::KEY_3;
       case GLFW_KEY_4: 		return InputAPI::Key::KEY_4;
       case GLFW_KEY_5: 		return InputAPI::Key::KEY_5;
       case GLFW_KEY_6: 		return InputAPI::Key::KEY_6;
       case GLFW_KEY_7: 		return InputAPI::Key::KEY_7;
       case GLFW_KEY_8: 		return InputAPI::Key::KEY_8;
       case GLFW_KEY_9: 		return InputAPI::Key::KEY_9;
       case GLFW_KEY_A: 		return InputAPI::Key::KEY_A;
       case GLFW_KEY_B: 		return InputAPI::Key::KEY_B;
       case GLFW_KEY_C: 		return InputAPI::Key::KEY_C;
       case GLFW_KEY_D: 		return InputAPI::Key::KEY_D;
       case GLFW_KEY_E: 		return InputAPI::Key::KEY_E;
       case GLFW_KEY_F: 		return InputAPI::Key::KEY_F;
       case GLFW_KEY_G: 		return InputAPI::Key::KEY_G;
       case GLFW_KEY_H: 		return InputAPI::Key::KEY_H;
       case GLFW_KEY_I: 		return InputAPI::Key::KEY_I;
       case GLFW_KEY_J: 		return InputAPI::Key::KEY_J;
       case GLFW_KEY_K: 		return InputAPI::Key::KEY_K;
       case GLFW_KEY_L: 		return InputAPI::Key::KEY_L;
       case GLFW_KEY_M: 		return InputAPI::Key::KEY_M;
       case GLFW_KEY_N: 		return InputAPI::Key::KEY_N;
       case GLFW_KEY_O: 		return InputAPI::Key::KEY_O;
       case GLFW_KEY_P: 		return InputAPI::Key::KEY_P;
       case GLFW_KEY_Q: 		return InputAPI::Key::KEY_Q;
       case GLFW_KEY_R: 		return InputAPI::Key::KEY_R;
       case GLFW_KEY_S: 		return InputAPI::Key::KEY_S;
       case GLFW_KEY_T: 		return InputAPI::Key::KEY_T;
       case GLFW_KEY_U: 		return InputAPI::Key::KEY_U;
       case GLFW_KEY_V: 		return InputAPI::Key::KEY_V;
       case GLFW_KEY_W: 		return InputAPI::Key::KEY_W;
       case GLFW_KEY_X: 		return InputAPI::Key::KEY_X;
       case GLFW_KEY_Y: 		return InputAPI::Key::KEY_Y;
       case GLFW_KEY_Z: 		return InputAPI::Key::KEY_Z;
       case GLFW_KEY_F1: 		return InputAPI::Key::KEY_F1;
       case GLFW_KEY_F2: 		return InputAPI::Key::KEY_F2;
       case GLFW_KEY_F3: 		return InputAPI::Key::KEY_F3;
       case GLFW_KEY_F4: 		return InputAPI::Key::KEY_F4;
       case GLFW_KEY_F5: 		return InputAPI::Key::KEY_F5;
       case GLFW_KEY_F6: 		return InputAPI::Key::KEY_F6;
       case GLFW_KEY_F7: 		return InputAPI::Key::KEY_F7;
       case GLFW_KEY_F8: 		return InputAPI::Key::KEY_F8;
       case GLFW_KEY_F9: 		return InputAPI::Key::KEY_F9;
       case GLFW_KEY_F10: 		return InputAPI::Key::KEY_F10;
       case GLFW_KEY_F11: 		return InputAPI::Key::KEY_F11;
       case GLFW_KEY_F12: 		return InputAPI::Key::KEY_F12;
       case GLFW_KEY_SPACE: 	return InputAPI::Key::KEY_SPACE;
       case GLFW_KEY_ESCAPE: 	return InputAPI::Key::KEY_ESCAPE;
       case GLFW_KEY_ENTER: 	return InputAPI::Key::KEY_ENTER;
       case GLFW_KEY_TAB: 		return InputAPI::Key::KEY_TAB;

	   case GLFW_KEY_UNKNOWN:
	   default:
       LOG_ERROR("Could not convert GLFW key ({}) to InputAPI::Key", pKeyInput);
	   return InputAPI::Key::KEY_UNKNOWN;
	}
}

InputAPI::MouseButton GLFWInput::convertMouseButton(const int& pMouseButton)
{
    switch (pMouseButton)
    {
      case GLFW_MOUSE_BUTTON_LEFT: 	    return InputAPI::MouseButton::MOUSE_LEFT;
      case GLFW_MOUSE_BUTTON_MIDDLE: 	return InputAPI::MouseButton::MOUSE_MIDDLE;
      case GLFW_MOUSE_BUTTON_RIGHT: 	return InputAPI::MouseButton::MOUSE_RIGHT;
      case GLFW_MOUSE_BUTTON_4: 	    return InputAPI::MouseButton::MOUSE_BUTTON_1;
      case GLFW_MOUSE_BUTTON_5: 	    return InputAPI::MouseButton::MOUSE_BUTTON_2;
      case GLFW_MOUSE_BUTTON_6: 	    return InputAPI::MouseButton::MOUSE_BUTTON_3;
      case GLFW_MOUSE_BUTTON_7: 	    return InputAPI::MouseButton::MOUSE_BUTTON_4;
      case GLFW_MOUSE_BUTTON_8: 	    return InputAPI::MouseButton::MOUSE_BUTTON_5;
      default:
          LOG_ERROR("Could not convert GLFW mouse button ({}) to InputAPI::MouseButton", pMouseButton);
          return InputAPI::MouseButton::MOUSE_UNKNOWN;
    }
}

InputAPI::Action GLFWInput::convertAction(const int& pAction)
{
    switch (pAction)
    {
      case GLFW_PRESS: 	        return InputAPI::Action::PRESS;
      case GLFW_RELEASE: 	    return InputAPI::Action::RELEASE;
      case GLFW_REPEAT: 	    return InputAPI::Action::REPEAT;
      default:
          LOG_ERROR("Could not convert GLFW action ({}) to InputAPI::Action", pAction);
          return InputAPI::Action::UNKNOWN;
    }
}