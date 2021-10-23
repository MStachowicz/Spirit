#include "Renderer.hpp"
#include "GLFW/glfw3.h"
#include <cstdlib>

namespace Renderer
{
	void init()
	{
		//glfwSetErrorCallback(error_callback);

		if (!glfwInit())
			exit(EXIT_FAILURE);

		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

		GLFWwindow *window = glfwCreateWindow(800, 600, "Zephyr", NULL, NULL);
		if (!window)
		{
			glfwTerminate();
			exit(EXIT_FAILURE);
		}

		while (true)
		{}
	}
}