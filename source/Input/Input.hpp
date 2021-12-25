#pragma once

class Context;
typedef struct GLFWwindow GLFWwindow;

// This is a fully static input class allowing it to be linked to GLFW's input callbacks
// which require a global or static implementation. The linkedGraphicsContext is the
// graphics API and window context the inputs will affect.
class Input
{
public:
    static void initialise(GLFWwindow *pWindow, Context* pLinkGraphicsContext);
    static void pollEvents();
    static bool closeRequested();

private:
    // Converts GLFW input to Input::Key
    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
    // The linked graphics context this input manager will manipulate.
    static Context* linkedGraphicsContext;
    static inline bool mClosing = false;

    enum class Key
    {
    KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,

    KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M,
    KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,

    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,

    KEY_SPACE, KEY_ESCAPE, KEY_ENTER, KEY_TAB,
    KEY_UNKNOWN
    };
    static Key convert(const int& pInput);
    static void onInput(const Key& pInput);
};