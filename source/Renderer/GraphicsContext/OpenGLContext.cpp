#include "OpenGLContext.hpp"
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "Logger.hpp"
#include "Input.hpp"

OpenGLContext::OpenGLContext()
: cOpenGLVersionMajor(3)
, cOpenGLVersionMinor(3)
, mWindow(nullptr)
, mGLADContext(nullptr)
{}

OpenGLContext::~OpenGLContext()
{
	shutdown();
}

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
		glfwMakeContextCurrent(mWindow);
		mGLADContext = (GladGLContext *)malloc(sizeof(GladGLContext));
		int version = gladLoadGLContext(mGLADContext, glfwGetProcAddress);
		if (!mGLADContext || version == 0)
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
		Input::linkedGraphicsContext = this;
		glfwSetWindowSizeCallback(mWindow, windowSizeCallback);
		glfwSetKeyCallback(mWindow, keyCallback);
		glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	LOG_INFO("OpenGL successfully initialised using GLFW and GLAD");
	return true;
}

bool OpenGLContext::isClosing()
{
	return glfwWindowShouldClose(mWindow);
}

void OpenGLContext::close() 
{
	glfwSetWindowShouldClose(mWindow, GL_TRUE);
}

void OpenGLContext::clearWindow()
{
	glfwMakeContextCurrent(mWindow);
	mGLADContext->Clear(GL_COLOR_BUFFER_BIT);
}

void OpenGLContext::swapBuffers()
{
	glfwSwapBuffers(mWindow);
}

void OpenGLContext::pollEvents()
{
	glfwPollEvents();
}

void OpenGLContext::shutdown()
{
	LOG_INFO("Shutting down OpenGLContext. Terminating GLFW and freeing GLAD memory.");
	glfwTerminate();
	if (mGLADContext)
		free(mGLADContext);
}

bool OpenGLContext::createWindow(const char *pName, int pWidth, int pHeight, bool pResizable)
{
	glfwWindowHint(GLFW_RESIZABLE, pResizable ? GL_TRUE : GL_FALSE);
	mWindow = glfwCreateWindow(pWidth, pHeight, pName, NULL, NULL);

	if (!mWindow)
	{
		LOG_WARN("Failed to create GLFW window");
		return false;
	}
	else
		return true;
}

void OpenGLContext::setClearColour(float pRed, float pGreen, float pBlue)
{
	glfwMakeContextCurrent(mWindow);
	mGLADContext->ClearColor(pRed / 255.0f, pGreen / 255.0f, pBlue / 255.0f, 1.0f);
}

void OpenGLContext::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_ESCAPE:
			Input::onInput(Input::InputType::Key_Escape);
		default:
			LOG_WARN("Cannot convert the GLFW key input to Zephyr input.");
			Input::onInput(Input::InputType::Unknown);
			break;
		}
	}
}

void OpenGLContext::windowSizeCallback(GLFWwindow *window, int width, int height)
{
	LOG_INFO("Window size changed to {}, {}", width, height);
	//glViewport(0, 0, width, height);
}