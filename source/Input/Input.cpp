#include "Input.hpp"
#include "Logger.hpp"
#include "Context.hpp"
#include "GLFW/glfw3.h"

Context* Input::linkedGraphicsContext = nullptr;

void Input::initialise(GLFWwindow *pWindow, Context* pLinkGraphicsContext)
{
    glfwSetKeyCallback(pWindow, keyCallback);
    glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    linkedGraphicsContext = pLinkGraphicsContext;
}

void Input::pollEvents()
{
    glfwPollEvents();
}

bool Input::closeRequested()
{
    return mClosing || linkedGraphicsContext ? linkedGraphicsContext->isClosing() : false;
}

// This function is linked to the GLFW key callback
void Input::onInput(const Key& pInput)
{
    switch (pInput)
    {
    case Key::KEY_W:
        //linkedActiveCamera->ProcessKeyboard(FORWARD);
        break;
    case Key::KEY_S:
        //linkedActiveCamera->ProcessKeyboard(BACKWARD);
        break;
    case Key::KEY_A:
        //linkedActiveCamera->ProcessKeyboard(LEFT);
        break;
    case Key::KEY_D:
        //linkedActiveCamera->ProcessKeyboard(RIGHT);
        break;
    case Key::KEY_ESCAPE:
        mClosing = true;
        linkedGraphicsContext->close();
        break;
    case Key::KEY_ENTER:
        linkedGraphicsContext->setClearColour(255, 255, 255);
        break;
    case Key::KEY_UNKNOWN:
    default:
        LOG_WARN("Unknown key press of value {}", pInput);
        break;
    }
}

void Input::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (action == GLFW_PRESS)
		onInput(convert(key));
}

Input::Key Input::convert(const int& pInput)
{
	switch (pInput)
	{
       case GLFW_KEY_0: 		return Input::Key::KEY_0;
       case GLFW_KEY_1: 		return Input::Key::KEY_1;
       case GLFW_KEY_2: 		return Input::Key::KEY_2;
       case GLFW_KEY_3: 		return Input::Key::KEY_3;
       case GLFW_KEY_4: 		return Input::Key::KEY_4;
       case GLFW_KEY_5: 		return Input::Key::KEY_5;
       case GLFW_KEY_6: 		return Input::Key::KEY_6;
       case GLFW_KEY_7: 		return Input::Key::KEY_7;
       case GLFW_KEY_8: 		return Input::Key::KEY_8;
       case GLFW_KEY_9: 		return Input::Key::KEY_9;
       case GLFW_KEY_A: 		return Input::Key::KEY_A;
       case GLFW_KEY_B: 		return Input::Key::KEY_B;
       case GLFW_KEY_C: 		return Input::Key::KEY_C;
       case GLFW_KEY_D: 		return Input::Key::KEY_D;
       case GLFW_KEY_E: 		return Input::Key::KEY_E;
       case GLFW_KEY_F: 		return Input::Key::KEY_F;
       case GLFW_KEY_G: 		return Input::Key::KEY_G;
       case GLFW_KEY_H: 		return Input::Key::KEY_H;
       case GLFW_KEY_I: 		return Input::Key::KEY_I;
       case GLFW_KEY_J: 		return Input::Key::KEY_J;
       case GLFW_KEY_K: 		return Input::Key::KEY_K;
       case GLFW_KEY_L: 		return Input::Key::KEY_L;
       case GLFW_KEY_M: 		return Input::Key::KEY_M;
       case GLFW_KEY_N: 		return Input::Key::KEY_N;
       case GLFW_KEY_O: 		return Input::Key::KEY_O;
       case GLFW_KEY_P: 		return Input::Key::KEY_P;
       case GLFW_KEY_Q: 		return Input::Key::KEY_Q;
       case GLFW_KEY_R: 		return Input::Key::KEY_R;
       case GLFW_KEY_S: 		return Input::Key::KEY_S;
       case GLFW_KEY_T: 		return Input::Key::KEY_T;
       case GLFW_KEY_U: 		return Input::Key::KEY_U;
       case GLFW_KEY_V: 		return Input::Key::KEY_V;
       case GLFW_KEY_W: 		return Input::Key::KEY_W;
       case GLFW_KEY_X: 		return Input::Key::KEY_X;
       case GLFW_KEY_Y: 		return Input::Key::KEY_Y;
       case GLFW_KEY_Z: 		return Input::Key::KEY_Z;
       case GLFW_KEY_F1: 		return Input::Key::KEY_F1;
       case GLFW_KEY_F2: 		return Input::Key::KEY_F2;
       case GLFW_KEY_F3: 		return Input::Key::KEY_F3;
       case GLFW_KEY_F4: 		return Input::Key::KEY_F4;
       case GLFW_KEY_F5: 		return Input::Key::KEY_F5;
       case GLFW_KEY_F6: 		return Input::Key::KEY_F6;
       case GLFW_KEY_F7: 		return Input::Key::KEY_F7;
       case GLFW_KEY_F8: 		return Input::Key::KEY_F8;
       case GLFW_KEY_F9: 		return Input::Key::KEY_F9;
       case GLFW_KEY_F10: 		return Input::Key::KEY_F10;
       case GLFW_KEY_F11: 		return Input::Key::KEY_F11;
       case GLFW_KEY_F12: 		return Input::Key::KEY_F12;
       case GLFW_KEY_SPACE: 	return Input::Key::KEY_SPACE;
       case GLFW_KEY_ESCAPE: 	return Input::Key::KEY_ESCAPE;
       case GLFW_KEY_ENTER: 	return Input::Key::KEY_ENTER;
       case GLFW_KEY_TAB: 		return Input::Key::KEY_TAB;

	   case GLFW_KEY_UNKNOWN:
	   default:
	   return Input::Key::KEY_UNKNOWN;
	}
}