#pragma once

typedef struct GLFWwindow GLFWwindow;

// Provides an OS window provided by GLFW. Wraps its construction and destruction.
// Handles Dear ImGui initialisation and rendering.
// Handles GLFW initialisation and termination.
class OpenGLWindow
{
	friend class OpenGLAPI;
public:
	OpenGLWindow(const OpenGLWindow &) = delete;
    OpenGLWindow(const int& pMajorVersion, const int& pMinorVersion, const int& pWidth = 1920, const int& pHeight = 1080, const bool& pResizable = true);
    ~OpenGLWindow();

    void swapBuffers();
    void startImGuiFrame();
    void renderImGui();

    static GLFWwindow* const getActiveWindowHandle(); // Allows Input to get the GLFW window to associate its input callbacks with.
private:
    GLFWwindow* mHandle;
    int mWidth;
    int mHeight;
    int mOpenGLMinorVersion;
    int mOpenGLMajorVersion;

    void onResize(const int& pNewWidth, const int& pNewHeight);


	static inline OpenGLWindow* currentWindow = nullptr; // Allows the static GLFW callback to call into this window and getActiveWindowHandle to function.
    static inline int activeGLFWWindows = 0; // Allows GLFW initialisation/termination to only happen on the first and last instances of a GLFWwindow.
};