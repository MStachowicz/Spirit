#include "GLFWInput.hpp"
#include "GLFW/glfw3.h"
#include "OpenGLWindow.hpp"
#include "Logger.hpp"

GLFWInput::GLFWInput()
{
    glfwSetInputMode(OpenGLWindow::getActiveWindowHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetKeyCallback(OpenGLWindow::getActiveWindowHandle(), keyCallback);
    glfwSetWindowCloseCallback(OpenGLWindow::getActiveWindowHandle(), windowCloseRequestCallback);
    currentActiveInputHandler = this;
};

void GLFWInput::pollEvents()
{
    glfwPollEvents();
};

bool GLFWInput::closeRequested()
{
    return mCloseRequested;
};

void GLFWInput::windowCloseRequestCallback(GLFWwindow* pWindow)
{
    currentActiveInputHandler->mCloseRequested = true;
}

void GLFWInput::keyCallback(GLFWwindow* pWindow, int pKey, int pScancode, int pAction, int pMode)
{
	if (pAction == GLFW_PRESS)
        currentActiveInputHandler->onKeyPress(convert(pKey));
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