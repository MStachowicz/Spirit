#include "Renderer.hpp"
#include "Logger.hpp"
#include "glad/gl.h"
#include "GLFW/glfw3.h"

namespace Renderer
{
	// Window dimensions
	const GLuint WIDTH = 400, HEIGHT = 300;	

	GladGLContext *create_context(GLFWwindow *window)
	{
		glfwMakeContextCurrent(window);

		GladGLContext *context = (GladGLContext *)malloc(sizeof(GladGLContext));
		if (!context)
			return NULL;

		int version = gladLoadGLContext(context, glfwGetProcAddress);
		LOG_INFO("Loaded OpenGL {}.{}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

		return context;
	}

	GLFWwindow *create_window(const char *name, int major, int minor)
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

		GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, name, NULL, NULL);

		if (window)
			LOG_INFO("Created a glfw window (OpenGL version {}.{})", major, minor);
		else
			LOG_WARN("Failed to create GLFW window");

		return window;
	}

	void draw(GLFWwindow *window, GladGLContext *gl, float r, float g, float b)
	{
		glfwMakeContextCurrent(window);

		gl->ClearColor(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
		gl->Clear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(window);
	}

	void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GL_TRUE);
	}

	void window_size_callback(GLFWwindow* window, int width, int height)
    {
		LOG_INFO("Window size changed to {}, {}", width, height);
    	//glViewport(0, 0, width, height);
    }

	bool init()
	{
		if (!glfwInit())
		{
			LOG_CRITICAL("GLFW initialisation failed");
			return false;
		}

		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

		GLFWwindow *window = create_window("Zephyr", 3, 3);

		if (!window)
		{
			LOG_CRITICAL("Base GLFW window creation failed. Terminating early");
			glfwTerminate();
			return false;
		}

		glfwSetKeyCallback(window, key_callback);
		glfwSetWindowSizeCallback(window, window_size_callback);
		glfwFocusWindow(window);
		glfwMakeContextCurrent(window);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		GladGLContext *context = create_context(window);
		if (!context)
		{
			LOG_CRITICAL("Failed to initialize GL context");
			free(context);
			return false;
		}

		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
			draw(window, context, 75.0f, 119.0f, 228.0f);
		}

		free(context);
		glfwTerminate();

		return true;
	}


}