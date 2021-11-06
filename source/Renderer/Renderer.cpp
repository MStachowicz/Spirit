#include "Renderer.hpp"
#include "Logger.hpp"
#include "glad/gl.h"
#include "GLFW/glfw3.h"

namespace Renderer
{
	GladGLContext *context 	= nullptr;
	GLFWwindow *mainWindow 	= nullptr;
	// OpenGL version set in glad_add_library() in CMakeLists.txt
	const int cOpenGLVersionMajor = 3, cOpenGLVersionMinor = 3; 

	void shutdown()
	{
		LOG_INFO("Shutting down Renderer. Terminating GLFW and freeing GLAD memory.");
		glfwTerminate();
		if (context)
			free(context);
	}

	GLFWwindow *createWindow(const char *pName, int pWidth, int pHeight, bool pResizable = true)
	{
		glfwWindowHint(GLFW_RESIZABLE, pResizable ? GL_TRUE : GL_FALSE);
		GLFWwindow *window = glfwCreateWindow(pWidth, pHeight, pName, NULL, NULL);

		if (!window)
			LOG_WARN("Failed to create GLFW window");

		return window;
	}

	// Set the clear colour for this window (RGB 0-255)
	void setClearColour(GLFWwindow * pWindow, float pRed, float pGreen, float pBlue)
	{
		glfwMakeContextCurrent(pWindow);
		context->ClearColor(pRed / 255.0f, pGreen / 255.0f, pBlue / 255.0f, 1.0f);
	}

	// Initialises OpenGL via GLAD using GLFW as the windowing context
	bool initialiseOpenGL()
	{
		{// Setup GLFW 
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

			mainWindow = createWindow("Zephyr", 1920, 1080);
			if (!mainWindow)
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

		LOG_INFO("OpenGL successfully initialised using GLFW and GLAD");
		return true;
	}

	void draw(GLFWwindow *window)
	{
		glfwMakeContextCurrent(window);
		context->Clear(GL_COLOR_BUFFER_BIT);

		// TODO: Perform drawing here...

		glfwSwapBuffers(window);
	}

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GL_TRUE);
	}

	void windowSizeCallback(GLFWwindow* window, int width, int height)
    {
		LOG_INFO("Window size changed to {}, {}", width, height);
    	//glViewport(0, 0, width, height);
    }

	bool initialise()
	{
		if (!initialiseOpenGL())
			return false;

		setClearColour(mainWindow, 75.0f, 119.0f, 228.0f);
		glfwFocusWindow(mainWindow);
		glfwSetWindowSizeCallback(mainWindow, windowSizeCallback);

		{// TODO: Move to input class or library
			glfwSetKeyCallback(mainWindow, keyCallback);
			glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}

		// Main loop, program terminates when the mainwindow is requested to be closed.
		while (!glfwWindowShouldClose(mainWindow))
		{
			glfwPollEvents();
			draw(mainWindow);
		}

		shutdown();
		return true;
	}
}