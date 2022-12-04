#pragma once

#include "EventDispatcher.hpp"

#include <utility>

typedef struct GLFWwindow GLFWwindow;
struct GladGLContext;

namespace Platform
{
    enum class Key;
    enum class MouseButton;
    enum class Action;
    enum class CursorMode;

    class Window
    {
        friend class Core;
    public:
        void setInputMode(const CursorMode& pCursorMode);
        void requestClose();

        bool capturingMouse()      const { return mCapturingMouse; };
        float aspectRatio()        const { return mAspectRatio; };
        std::pair<int, int> size() const { return {mWidth, mHeight}; };

    private:
        Window(const int& pWidth, const int& pHeight);
        ~Window();

        int mWidth;
        int mHeight;
        float mAspectRatio;
        bool mCapturingMouse; // Is the mouse hidden and input is restricted to this window.
        GLFWwindow* mHandle;
    };


    // Core abstracts away any external libs we are using.
    // Core provides a platform agnostic handler for Windows, Input and Graphics contexts.
    class Core
    {
    public:
        // HELPERS
        static std::pair<int,int> getPrimaryMonitorResolution();

        // INIT
        static void initialise();
        static void cleanup();

        // GRAPHICS
        static void swapBuffers();
        static void startImGuiFrame();
        static void endImGuiFrame();
        static Window& getWindow() { return *mPrimaryWindow; };
        static bool hasWindow()    { return mPrimaryWindow;  };

        // INPUT
        static void pollEvents();
        static bool UICapturingMouse();

        // EVENTS
        static inline Utility::EventDispatcher<const Key&> mKeyPressEvent;
        static inline Utility::EventDispatcher<const MouseButton&, const Action&> mMouseButtonEvent;
        static inline Utility::EventDispatcher<const float&, const float&> mMouseMoveEvent;
        static inline Utility::EventDispatcher<const int&, const int&> mWindowResizeEvent;

    private:
        static inline Window* mPrimaryWindow = nullptr;
        static inline GladGLContext* mOpenGLContext;
        static inline double mLastXPosition = -1.0;
        static inline double mLastYPosition = -1.0;

        // GLFW callbacks
        static void GLFW_errorCallback(int pError, const char* pDescription);
        static void GLFW_windowCloseCallback(GLFWwindow* pWindow); // Called when the window title bar X is pressed.
        static void GLFW_keyPressCallback(GLFWwindow* pWindow, int pKey, int pScancode, int pAction, int pMode); // Called when a key is pressed in glfwPollEvents.
        static void GLFW_mouseMoveCallback(GLFWwindow* pWindow, double pNewXPosition, double pNewYPosition);
        static void GLFW_mouseButtonCallback(GLFWwindow* pWindow, int pButton, int pAction, int pModifiers);
        static void GLFW_windowResizeCallback(GLFWwindow* pWindow, int pWidth, int pHeight);

        // GLFW to platform conversion funcs
        static Key GLFW_getKey(const int& pKeyInput);
        static MouseButton GLFW_getMouseButton(const int& pMouseButton);
        static Action GLFW_getAction(const int& pAction);
    };
}