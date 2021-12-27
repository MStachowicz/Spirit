#pragma once

typedef struct GLFWwindow GLFWwindow;

// Input handler using GLFW. Must be initialised after an OpenGLWindow as GLFW callbacks require a window context.
class Input
{
public:
    void initialise();
    void pollEvents();
    bool closeRequested();

private:
    bool mClosing = false;

    enum class Key
    {
        KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,

        KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M,
        KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,

        KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,

        KEY_SPACE, KEY_ESCAPE, KEY_ENTER, KEY_TAB,
        KEY_UNKNOWN
    };
    void onInput(const Key& pInput);

    static void keyCallback(GLFWwindow* pWindow, int pKey, int pScancode, int pAction, int pMode); // GLFW callback when a key is pressed.
    static void windowCloseRequestCallback(GLFWwindow* pWindow); // GLFW callback when a window is closed via title bar or other non-application means.
    static Key convert(const int& pGLFWKey); // Converts GLFW input to Input::Keys.
    static inline Input* currentInputHandler = nullptr; // Exists to give static GLFW callbacks an instance to use.
};