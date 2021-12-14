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
	: cOpenGLVersionMajor(3), cOpenGLVersionMinor(3), cGLSLVersion("#version 330"), cMaxTextureUnits(2), mShaderProgram(0), mTextureShader(0), mWindow(nullptr), mGLADContext(nullptr)
{}

OpenGLContext::~OpenGLContext()
{
	LOG_INFO("Shutting down OpenGLContext. Terminating GLFW and freeing GLAD memory.");
	glfwTerminate();
	if (mGLADContext)
		free(mGLADContext);

	shutdownImGui();
}

bool OpenGLContext::initialise()
{
	{ // Setup GLFW
		if (!glfwInit())
		{
			LOG_CRITICAL("GLFW initialisation failed");
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

			return false;
		}

		LOG_INFO("Loaded OpenGL {}.{} using GLAD", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
		// TODO: Add an assert here for GLAD_VERSION to equal to cOpenGLVersion
	}

	{ // Setup GLFW callbacks for input and window changes
		mGLADContext->Viewport(0, 0, 1920, 1080);
		Input::linkedGraphicsContext = this;
		glfwSetWindowSizeCallback(mWindow, windowSizeCallback);
		glfwSetKeyCallback(mWindow, keyCallback);
		glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	initialiseShaders();
	initialiseTextures();
	initialiseImGui();

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

void OpenGLContext::draw(const Mesh &pMesh)
{
	const auto &it = mMeshManager.find(pMesh.mID);

	if (it != mMeshManager.end())
		draw(it->second);
}

void OpenGLContext::draw(const DrawInfo &pDrawInfo)
{
	glUseProgram(pDrawInfo.mShaderID);
	glPolygonMode(GL_FRONT_AND_BACK, pDrawInfo.mPolygonMode);
	glBindVertexArray(pDrawInfo.mVAO);

	for (size_t i = 0; i < pDrawInfo.mTextures.size(); ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, pDrawInfo.mTextures[i]);
	}

	if (pDrawInfo.mDrawMethod == DrawInfo::DrawMethod::Indices)
		glDrawElements(pDrawInfo.mDrawMode, static_cast<GLsizei>(pDrawInfo.mDrawSize), GL_UNSIGNED_INT, 0);
	else if (pDrawInfo.mDrawMethod == DrawInfo::DrawMethod::Array)
		glDrawArrays(pDrawInfo.mDrawMode, 0, static_cast<GLsizei>(pDrawInfo.mDrawSize));
}

void OpenGLContext::setHandle(Mesh &pMesh)
{
	ZEPHYR_ASSERT(!pMesh.mVertices.empty(), "Cannot set a mesh handle for a mesh with no position data.")

	const auto &it = mMeshManager.find(pMesh.mID);
	ZEPHYR_ASSERT(it == mMeshManager.end(), "Calling setHandle on a mesh already present in the mesh manager. This mesh is already initialised.")

	DrawInfo drawInfo;
	drawInfo.mDrawMode = GL_TRIANGLES; //OpenGLContext only supports GL_TRIANGLES at this revision
	drawInfo.mPolygonMode = GL_FILL;   //Wireframe = 0x1B01
	drawInfo.mDrawMethod = pMesh.mIndices.empty() ? DrawInfo::DrawMethod::Array : DrawInfo::DrawMethod::Indices;
	drawInfo.mDrawSize = pMesh.mIndices.empty() ? pMesh.mVertices.size() : pMesh.mIndices.size();
	drawInfo.mShaderID = pMesh.mTextureCoordinates.empty() ? mShaderProgram : mTextureShader;

	glUseProgram(drawInfo.mShaderID);
	glGenVertexArrays(1, &drawInfo.mVAO);
	glBindVertexArray(drawInfo.mVAO);

	// Per vertex attributes
	{ // POSITIONS
		unsigned int VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, pMesh.mVertices.size() * sizeof(float), &pMesh.mVertices.front(), GL_STATIC_DRAW);
		const GLint attributeIndex = glGetAttribLocation(drawInfo.mShaderID, "VertexPosition");
		ZEPHYR_ASSERT(attributeIndex != -1, "Failed to find the location of VertexPosition in shader program with ID {}.", drawInfo.mShaderID);
		glVertexAttribPointer(attributeIndex, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(attributeIndex);
	} // Remaining data is optional:

	if (!pMesh.mIndices.empty())
	{ // INDICES (Element buffer - re-using data)
		unsigned int EBO;
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, pMesh.mIndices.size() * sizeof(int), &pMesh.mIndices.front(), GL_STATIC_DRAW);
	}

	if (!pMesh.mColours.empty())
	{ // COLOURS
		ZEPHYR_ASSERT(pMesh.mColours.size() == pMesh.mVertices.size(), ("Size of colour data ({}) does not match size of position data ({}), cannot buffer the colour data", pMesh.mColours.size(), pMesh.mVertices.size()));

		unsigned int VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, pMesh.mColours.size() * sizeof(float), &pMesh.mColours.front(), GL_STATIC_DRAW);
		const GLint attributeIndex = glGetAttribLocation(drawInfo.mShaderID, "VertexColour");
		ZEPHYR_ASSERT(attributeIndex != -1, "Failed to find the location of VertexColour in shader program with ID {}.", drawInfo.mShaderID);
		glVertexAttribPointer(attributeIndex, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(attributeIndex);
	}
	if (!pMesh.mTextureCoordinates.empty())
	{ // TEXTURE COORDINATES
		unsigned int VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, pMesh.mTextureCoordinates.size() * sizeof(float), &pMesh.mTextureCoordinates.front(), GL_STATIC_DRAW);
		const GLint attributeIndex = glGetAttribLocation(drawInfo.mShaderID, "VertexTexCoord");
		ZEPHYR_ASSERT(attributeIndex != -1, "Failed to find the location of VertexTexCoord in shader program with ID {}.", drawInfo.mShaderID);
		glVertexAttribPointer(attributeIndex, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(attributeIndex);

		ZEPHYR_ASSERT(pMesh.mTextures.size() <= cMaxTextureUnits, "This mesh has been assigned more textures than the context currently allows.");
		// Assign the ID's of the textures requested for this Mesh
		for (const std::string &texture : pMesh.mTextures)
		{
			const auto it = mTextures.find(texture);
			ZEPHYR_ASSERT(it != mTextures.end(), "Texture {} doesn't exist and could not be loaded for this mesh.", texture)
			drawInfo.mTextures.push_back(it->second);
		}
	}

	mMeshManager.insert({pMesh.mID, drawInfo});
}

void OpenGLContext::setClearColour(const float &pRed, const float &pGreen, const float &pBlue)
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

bool OpenGLContext::initialiseImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(mWindow, true);
	ImGui_ImplOpenGL3_Init(cGLSLVersion.c_str());
	return true;
}

void OpenGLContext::shutdownImGui()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
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

void OpenGLContext::initialiseTextures()
{
	{ // Load all the textures in the textures directory
		std::vector<std::string> textureFileNames = File::getAllFileNames(File::textureDirectory);

		for (size_t i = 0; i < textureFileNames.size(); ++i)
			mTextures.insert({textureFileNames[i], loadTexture(textureFileNames[i])});
	}

	{ // Setup the available texture units. These map the uniform sampler2D slots found in the shader to texture units
		glUseProgram(mTextureShader);
		for (size_t i = 0; i < cMaxTextureUnits; ++i)
		{
			std::string textureUniformName = "texture" + std::to_string(i);
			glUniform1i(glGetUniformLocation(mTextureShader, textureUniformName.c_str()), i);
		}
	}
}

unsigned int OpenGLContext::loadTexture(const std::string &pFileName)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	File::Texture texture = File::getTexture(pFileName);

	const int channelType = texture.mNumberOfChannels == 4 ? GL_RGBA : GL_RGB;
	glTexImage2D(GL_TEXTURE_2D, 0, channelType, texture.mWidth, texture.mHeight, 0, channelType, GL_UNSIGNED_BYTE, texture.mData);
	glGenerateMipmap(GL_TEXTURE_2D);
	ZEPHYR_ASSERT(textureID != -1, "Texture {} failed to load", pFileName);
	LOG_INFO("Texture {} loaded given ID: {}", pFileName, textureID);

	return textureID;
}

void OpenGLContext::initialiseShaders()
{
	mShaderProgram = loadShader(File::shaderDirectory + "triangle.vert", File::shaderDirectory + "triangle.frag");
	mTextureShader = loadShader(File::shaderDirectory + "texture.vert", File::shaderDirectory + "texture.frag");
}

unsigned int OpenGLContext::loadShader(const std::string &pVertexShader, const std::string &pFragmentShader)
{
	unsigned int vertexShader;
	{
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		std::string source = File::readFromFile(pVertexShader);
		const char *vertexShaderSource = source.c_str();
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);
		ZEPHYR_ASSERT(!hasCompileErrors(vertexShader, ProgramType::VertexShader), "Failed to compile vertex shader with path {}", pVertexShader)
	}

	unsigned int fragmentShader;
	{
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		std::string source = File::readFromFile(pFragmentShader);
		const char *fragmentShaderSource = source.c_str();
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);
		ZEPHYR_ASSERT(!hasCompileErrors(fragmentShader, ProgramType::FragmentShader), "Failed to compile fragment shader with path {}", pFragmentShader)
	}

	unsigned int shaderProgram;
	{
		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);
		ZEPHYR_ASSERT(!hasCompileErrors(shaderProgram, ProgramType::ShaderProgram), "Failed to compile fragment shader with path {}", pFragmentShader)
	}

	// Delete the shaders after linking as they're no longer needed
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	LOG_INFO("Shader program {} loaded using vertex shader {} and fragment shader {}", shaderProgram, pVertexShader, pFragmentShader);
	return shaderProgram;
}

bool OpenGLContext::hasCompileErrors(const unsigned int pProgramID, const ProgramType &pType)
{
	int success;
	if (pType == ProgramType::ShaderProgram)
	{
		glGetProgramiv(pProgramID, GL_LINK_STATUS, &success);
		if (!success)
		{
			char infoLog[1024];
			glGetProgramInfoLog(pProgramID, 1024, NULL, infoLog);
			LOG_ERROR("Program linking failed with info: {}", infoLog);
			return true;
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
			return true;
		}
	}

	return false;
}

void OpenGLContext::pollEvents()
{
	glfwPollEvents();
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