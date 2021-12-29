#pragma once

#include "InputAPI.hpp"

typedef struct GLFWwindow GLFWwindow;

// Implements InputAPI using GLFW.
// GLFWInput requires a valid GLFW context to be initialised before and a GLFWwindow* instance to register callbacks.
class GLFWInput : public InputAPI
{
public:
    void initialise() override; // Registers the static GLFW callback functions. Sets this instance as currentActiveInputHandler.
    void pollEvents() override;
    bool closeRequested() override;

private:
    bool mCloseRequested = false;

    // The remaining functions are required to be static by GLFW for callback
    // ------------------------------------------------------------------------------------------------------------------------------
    static inline GLFWInput* currentActiveInputHandler = nullptr; // The instance of GLFWInput called in the static GLFW callbacks.
    static void windowCloseRequestCallback(GLFWwindow* pWindow); // Called when the window title bar X is pressed.
    static void keyCallback(GLFWwindow* pWindow, int pKey, int pScancode, int pAction, int pMode); // Called when a key is pressed in glfwPollEvents.
    static InputAPI::Key convert(const int& pKeyInput);
};