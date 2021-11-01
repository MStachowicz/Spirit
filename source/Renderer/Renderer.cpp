#include "Renderer.hpp"
#include "GLFW/glfw3.h"
#include "Logger.hpp"

namespace Renderer
{
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

		return true;
	}
}