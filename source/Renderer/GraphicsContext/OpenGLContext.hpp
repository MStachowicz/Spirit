#include "Context.hpp"

#pragma once

typedef struct GLFWwindow GLFWwindow;
struct GladGLContext;

// Implements OpenGL API via GLAD2 and binds it to mWindow provided by GLFW.
// GLFW also provides input functionality which is set to callback to static functions
// keyCallback() and windowSizeCallback()
class OpenGLContext : public Context
{
public:
	OpenGLContext();
	~OpenGLContext();

	// Inherited from Context.hpp
	bool initialise() 	override;
	bool isClosing() 	override;
	void close() 		override;
	void clearWindow() 	override;
	void swapBuffers() 	override;
	void pollEvents() 	override;
	void setClearColour(const float& pRed, const float& pGreen, const float& pBlue) override;
	void newImGuiFrame() override;
	void renderImGuiFrame() override;

protected:
	bool initialiseImGui() 	override;
	void shutdownImGui() 	override;

private:
	// OpenGL version set in glad_add_library() in CMakeLists.txt
	const int 		cOpenGLVersionMajor, cOpenGLVersionMinor;
	const char*		cGLSLVersion;
	unsigned int	mShaderProgram;
	GLFWwindow* 	mWindow;
	GladGLContext* 	mGLADContext;

	void shutdown();
	bool createWindow(const char *pName, int pWidth, int pHeight, bool pResizable = true);
	bool initialiseShaderProgram();

	// Callbacks required by GLFW to be static/global
	static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
	static void windowSizeCallback(GLFWwindow *window, int width, int height);
};