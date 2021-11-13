#include "OpenGLContext.hpp"
#include "Logger.hpp"
#include "glad/gl.h"
#include "GLFW/glfw3.h"

// Globally defined members and functions allowing the header for 
// OpenGLContext to not include GLFW and GLAD.
// ---------------------------------------------------------------------

GLFWwindow *mainWindow = nullptr;
GladGLContext *context = nullptr;

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void windowSizeCallback(GLFWwindow *window, int width, int height)
{
	LOG_INFO("Window size changed to {}, {}", width, height);
	//glViewport(0, 0, width, height);
}
// ---------------------------------------------------------------------

OpenGLContext::~OpenGLContext()
{
	shutdown();
}

// Initialises OpenGL via GLAD using GLFW as the windowing context
bool OpenGLContext::initialise()
{
	{ // Setup GLFW
		if (!glfwInit())
		{
			LOG_CRITICAL("GLFW initialisation failed");
			shutdown();
			return false;
		}

		LOG_INFO("Initialised GLFW successfully");
	}

	{ // Create a GLFW window for GLAD setup
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, cOpenGLVersionMajor);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, cOpenGLVersionMinor);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		if (!createWindow("Zephyr", 1920, 1080))
		{
			LOG_CRITICAL("Base GLFW window creation failed. Terminating early");
			shutdown();
			return false;
		}

		LOG_INFO("Main GLFW window created successfully");
	}

	{ // Setup GLAD
		glfwMakeContextCurrent(mainWindow);
		context = (GladGLContext *)malloc(sizeof(GladGLContext));
		int version = gladLoadGLContext(context, glfwGetProcAddress);
		if (!context || version == 0)
		{
			LOG_CRITICAL("Failed to initialise GLAD GL context");

			if (!glfwGetCurrentContext())
				LOG_ERROR("No window was set as current context. Call glfwMakeContextCurrent before gladLoadGLContext");

			shutdown();
			return false;
		}

		LOG_INFO("Loaded OpenGL using GLAD with version {}.{}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
		// TODO: Add an assert here for GLAD_VERSION to equal to cOpenGLVersion
	}

	{ // Setup GLFW callbacks for input and window changes
		glfwSetWindowSizeCallback(mainWindow, windowSizeCallback);
		glfwSetKeyCallback(mainWindow, keyCallback);
		glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	LOG_INFO("OpenGL successfully initialised using GLFW and GLAD");
	return true;
}

bool OpenGLContext::isClosing()
{
	return glfwWindowShouldClose(mainWindow);
}

void OpenGLContext::clearWindow()
{
	glfwMakeContextCurrent(mainWindow);
	context->Clear(GL_COLOR_BUFFER_BIT);
}

void OpenGLContext::swapBuffers()
{
	glfwSwapBuffers(mainWindow);
}

void OpenGLContext::pollEvents()
{
	glfwPollEvents();
}

void OpenGLContext::shutdown()
{
	LOG_INFO("Shutting down OpenGLContext. Terminating GLFW and freeing GLAD memory.");
	glfwTerminate();
	if (context)
		free(context);
}

bool OpenGLContext::createWindow(const char *pName, int pWidth, int pHeight, bool pResizable)
{
	glfwWindowHint(GLFW_RESIZABLE, pResizable ? GL_TRUE : GL_FALSE);
	mainWindow = glfwCreateWindow(pWidth, pHeight, pName, NULL, NULL);

	if (!mainWindow)
	{
		LOG_WARN("Failed to create GLFW window");
		return false;
	}
	else
		return true;
}

// Set the clear colour for this window (RGB 0-255)
void OpenGLContext::setClearColour(float pRed, float pGreen, float pBlue)
{
	glfwMakeContextCurrent(mainWindow);
	context->ClearColor(pRed / 255.0f, pGreen / 255.0f, pBlue / 255.0f, 1.0f);
}