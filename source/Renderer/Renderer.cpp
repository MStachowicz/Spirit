#include "Renderer.hpp"
#include "GLFW/glfw3.h"
#include "Logger.hpp"

namespace Renderer
{
    void windowSizeChanged(GLFWwindow* window, int pWidth, int pHeight)
    {
		LOG_INFO("Window size changed to {}, {}", pWidth, pHeight);
    	//glViewport(0, 0, width, height);
    }
    
	bool init()
	{
		if (!glfwInit())
		{
			LOG_CRITICAL("GLFW initialisation failed");
			return false;
		}

		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

		GLFWwindow *window = glfwCreateWindow(800, 600, "Zephyr", NULL, NULL);
		if (!window)
		{
			LOG_CRITICAL("GLFW window creation failed");
			glfwTerminate();
			return false;
		}

		glfwFocusWindow(window);
		glfwMakeContextCurrent(window);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		glfwSetFramebufferSizeCallback(window, Renderer::windowSizeChanged);

		return true;
	}


}