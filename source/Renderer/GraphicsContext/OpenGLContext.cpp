#include "OpenGLContext.hpp"
// The imgui headers must be included before the graphics context
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_draw.cpp"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Logger.hpp"
#include "Input.hpp"
#include "FileSystem.hpp"

OpenGLContext::OpenGLContext()
: cOpenGLVersionMajor(3)
, cOpenGLVersionMinor(3)
, cGLSLVersion("#version 330")
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
		mGLADContext->Viewport(0, 0, 1920, 1080);
		Input::linkedGraphicsContext = this;
		glfwSetWindowSizeCallback(mWindow, windowSizeCallback);
		glfwSetKeyCallback(mWindow, keyCallback);
		glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	{// Initialise shaders
		if (!initialiseShaderProgram())
		{
			LOG_ERROR("Shader program failed to initialise");
			return false;
		}
		LOG_INFO("Shader program loaded successfully with ID: {}", mShaderProgram);
		glUseProgram(mShaderProgram);
	}

	{ // Setup ImGui
		initialiseImGui();
	}

	LOG_INFO("OpenGL successfully initialised using GLFW and GLAD");
	return true;
}

bool OpenGLContext::initialiseImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(mWindow, true);
	ImGui_ImplOpenGL3_Init(cGLSLVersion);
	return true;
}

void OpenGLContext::shutdownImGui()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
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

void OpenGLContext::draw(const Mesh& pMesh)
{
	glPolygonMode(GL_FRONT_AND_BACK, castToUnderlyingType(pMesh.mPolygonMode));
	glBindVertexArray(pMesh.mVAO);

	if (!pMesh.mIndices.empty())
		glDrawElements(castToUnderlyingType(pMesh.mDrawMode), static_cast<GLsizei>(pMesh.mIndices.size()), GL_UNSIGNED_INT, 0);
	else
		glDrawArrays(castToUnderlyingType(pMesh.mDrawMode), 0, static_cast<GLsizei>(pMesh.mVertices.size()));
}

void OpenGLContext::setHandle(Mesh &pMesh)
{
	if (!pMesh.mVertices.empty())
	{
		pMesh.mDrawMode = Mesh::DrawMode::Triangles;
		glGenVertexArrays(1, &pMesh.mVAO);
		glBindVertexArray(pMesh.mVAO);

		if (!pMesh.mIndices.empty())
		{ // INDICES (Element buffer - re-using data)
			unsigned int EBO;
			glGenBuffers(1, &EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, pMesh.mIndices.size() * sizeof(int), &pMesh.mIndices.front(), GL_STATIC_DRAW);
		}

		// Per vertex attributes
		{ // POSITIONS
			unsigned int VBO;
			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, pMesh.mVertices.size() * sizeof(float), &pMesh.mVertices.front(), GL_STATIC_DRAW);
			const GLint attributeIndex = glGetAttribLocation(mShaderProgram, "VertexPosition");
			ZEPHYR_ASSERT(attributeIndex != -1, "Failed to find the location of position attribute in shader program. Attribute string searched for: 'VertexPosition' in shader program with ID: '{}'", mShaderProgram);
			glVertexAttribPointer(attributeIndex, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
			glEnableVertexAttribArray(attributeIndex);
		}
		{ // COLOURS
			ZEPHYR_ASSERT(pMesh.mColours.size() == pMesh.mVertices.size(), ("Size of colour data ({}) does not match size of position data ({}), cannot buffer the colour data", pMesh.mColours.size(), pMesh.mVertices.size()));

			unsigned int VBO;
			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, pMesh.mColours.size() * sizeof(float), &pMesh.mColours.front(), GL_STATIC_DRAW);
			const GLint attributeIndex = glGetAttribLocation(mShaderProgram, "VertexColour");
			ZEPHYR_ASSERT(attributeIndex != -1, "Failed to find the location of position attribute in shader program. Attribute string searched for: 'VertexColour' in shader program with ID: '{}'", mShaderProgram);
			glVertexAttribPointer(attributeIndex, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
			glEnableVertexAttribArray(attributeIndex);
		}
	}
}

void OpenGLContext::setClearColour(const float& pRed, const float& pGreen, const float& pBlue)
{
	glfwMakeContextCurrent(mWindow);
	mGLADContext->ClearColor(pRed / 255.0f, pGreen / 255.0f, pBlue / 255.0f, 1.0f);
}

void OpenGLContext::newImGuiFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void OpenGLContext::renderImGuiFrame()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void OpenGLContext::shutdown()
{
	LOG_INFO("Shutting down OpenGLContext. Terminating GLFW and freeing GLAD memory.");
	glfwTerminate();
	if (mGLADContext)
		free(mGLADContext);

	shutdownImGui();
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

enum ProgramType
{
	VertexShader, FragmentShader, ShaderProgram
};

bool checkCompileErrors(const unsigned int pProgramID, const ProgramType& pType)
{
	int success;
	if (pType == ShaderProgram)
	{
		glGetProgramiv(pProgramID, GL_LINK_STATUS, &success);
		if (!success)
		{
			char infoLog[1024];
			glGetProgramInfoLog(pProgramID, 1024, NULL, infoLog);
			LOG_ERROR("Program linking failed with info: {}", infoLog);
			return false;
		}
	}
	else
	{
		glGetShaderiv(pProgramID, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			char infoLog[1024];
			glGetShaderInfoLog(pProgramID, 1024, NULL, infoLog);
			LOG_ERROR("Shader compilation failed with info: {}", infoLog);
			return false;
		}
	}

	return true;
}

bool OpenGLContext::initialiseShaderProgram()
{
	unsigned int vertexShader;
	{
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		std::string path = File::shaderDirectory + "triangle.vert";
		std::string source = File::readFromFile(path);
		const char* vertexShaderSource = source.c_str();
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);
		if (!checkCompileErrors(vertexShader, VertexShader))
			return false;
	}

	unsigned int fragmentShader;
	{
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		std::string source = File::readFromFile(File::shaderDirectory + "triangle.frag");
		const char* fragmentShaderSource = source.c_str();
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);
		if (!checkCompileErrors(fragmentShader, FragmentShader))
			return false;
	}

	unsigned int shaderProgram;
	{
		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);
		if (!checkCompileErrors(shaderProgram, ShaderProgram))
			return false;
	}

	// Delete the shaders after linking as they're no longer needed
	glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
	mShaderProgram = shaderProgram;
	return true;
}

void OpenGLContext::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (action == GLFW_PRESS)
		Input::onInput(key);
}

void OpenGLContext::windowSizeCallback(GLFWwindow *window, int width, int height)
{
	LOG_INFO("Window size changed to {}, {}", width, height);
	glViewport(0, 0, width, height);
}