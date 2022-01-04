#include "OpenGLAPI.hpp"

#include "FileSystem.hpp"

#include "glad/gl.h"
#include "GLFW/glfw3.h" // Used to initialise GLAD using glfwGetProcAddress

#include "glm/ext/matrix_transform.hpp" // perspective, translate, rotate
#include "glm/gtc/type_ptr.hpp"

#include "imgui.h"

OpenGLAPI::OpenGLAPI()
	: cOpenGLVersionMajor(3)
	, cOpenGLVersionMinor(3)
	, cMaxTextureUnits(2)
	, mWindow(cOpenGLVersionMajor, cOpenGLVersionMinor)
	, mGLADContext(initialiseGLAD())
	, mWindowClearColour{0.0f, 0.0f, 0.0f}
	, mTextureShader("texture")
{
    glfwSetWindowSizeCallback(mWindow.mHandle, windowSizeCallback);
	glViewport(0, 0, mWindow.mWidth, mWindow.mHeight);

	glEnable(GL_DEPTH_TEST);

	initialiseTextures();
	buildMeshes();

	LOG_INFO("OpenGL successfully initialised using GLFW and GLAD");
}

OpenGLAPI::~OpenGLAPI()
{
	if (mGLADContext)
	{
		free(mGLADContext);
		LOG_INFO("OpenGLAPI destructor called. Freeing GLAD memory.");
	}
}

void OpenGLAPI::clearBuffers()
{
	mGLADContext->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLAPI::setView(const glm::mat4 &pViewMatrix)
{
	mViewMatrix = pViewMatrix;
};

void OpenGLAPI::onFrameStart()
{
	clearBuffers();
	mWindow.startImGuiFrame();

	if (ImGui::Begin("OpenGL options"))
	{
		if(ImGui::ColorEdit3("Window clear colour", mWindowClearColour))
			setClearColour(mWindowClearColour[0], mWindowClearColour[1], mWindowClearColour[2]);
	}
	ImGui::End();
}

void OpenGLAPI::draw()
{
	for (size_t i = 0; i < mDrawQueue.size(); i++)
	{
		// Grab the DrawInfo for the Mesh requested.
		const DrawInfo& drawInfo = getDrawInfo(mDrawQueue[i].mMesh);
		mTextureShader.use();

		glm::mat4 trans = glm::translate(glm::mat4(1.0f), mDrawQueue[i].mPosition);
		trans = glm::rotate(trans, glm::radians(mDrawQueue[i].mRotation.x), glm::vec3(1.0, 0.0, 0.0));
		trans = glm::rotate(trans, glm::radians(mDrawQueue[i].mRotation.y), glm::vec3(0.0, 1.0, 0.0));
		trans = glm::rotate(trans, glm::radians(mDrawQueue[i].mRotation.z), glm::vec3(0.0, 0.0, 1.0));
		trans = glm::scale(trans, mDrawQueue[i].mScale);
		mTextureShader.setUniform("model", trans);

		// note that we're translating the scene in the reverse direction of where we want to move
		glm::mat4 projection 	= glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
		mTextureShader.setUniform("view", mViewMatrix);
		mTextureShader.setUniform("projection", projection);

		glPolygonMode(GL_FRONT_AND_BACK, getPolygonMode(mDrawQueue[i].mDrawMode));

		glBindVertexArray(drawInfo.mVAO);

		if (mDrawQueue[i].mTexture.has_value())
		{
			mTextureShader.setUniform("useTextures", true);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mDrawQueue[i].mTexture.value());
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else
			mTextureShader.setUniform("useTextures", false);


		if (drawInfo.mDrawMethod == DrawInfo::DrawMethod::Indices)
			glDrawElements(drawInfo.mDrawMode, static_cast<GLsizei>(drawInfo.mDrawSize), GL_UNSIGNED_INT, 0);
		else if (drawInfo.mDrawMethod == DrawInfo::DrawMethod::Array)
			glDrawArrays(drawInfo.mDrawMode, 0, static_cast<GLsizei>(drawInfo.mDrawSize));
	}
	mDrawQueue.clear();

	mWindow.renderImGui();
	mWindow.swapBuffers();
}

const OpenGLAPI::DrawInfo& OpenGLAPI::getDrawInfo(const MeshID& pMeshID)
{
	const auto it = mMeshManager.find(pMeshID);
	ZEPHYR_ASSERT(it != mMeshManager.end(), "Mesh ID does not exist in the mesh manager. Cannot return the draw info.");
	return it->second;
}

void OpenGLAPI::initialiseMesh(const Mesh &pMesh)
{
	ZEPHYR_ASSERT(!pMesh.mVertices.empty(), "Cannot set a mesh handle for a mesh with no position data.")

	const auto &it = mMeshManager.find(pMesh.mID);
	ZEPHYR_ASSERT(it == mMeshManager.end(), "Calling initialiseMesh on a mesh already present in the mesh manager. This mesh is already initialised.")

	DrawInfo drawInfo 		= DrawInfo(mTextureShader);
	drawInfo.mDrawMode 		= GL_TRIANGLES; //OpenGLAPI only supports GL_TRIANGLES at this revision
	drawInfo.mDrawMethod 	= pMesh.mIndices.empty() ? DrawInfo::DrawMethod::Array : DrawInfo::DrawMethod::Indices;
	drawInfo.mDrawSize 		= pMesh.mIndices.empty() ? static_cast<int>(pMesh.mVertices.size()) : static_cast<int>(pMesh.mIndices.size());

	drawInfo.mShaderID.use();
	glGenVertexArrays(1, &drawInfo.mVAO);
	glBindVertexArray(drawInfo.mVAO);

	// Per vertex attributes
	{ // POSITIONS
		unsigned int VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, pMesh.mVertices.size() * sizeof(float), &pMesh.mVertices.front(), GL_STATIC_DRAW);

		const GLint attributeIndex = static_cast<GLint>(drawInfo.mShaderID.getAttributeLocation("VertexPosition"));
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
		const GLint attributeIndex = static_cast<GLint>(drawInfo.mShaderID.getAttributeLocation("VertexColour"));
		glVertexAttribPointer(attributeIndex, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(attributeIndex);
	}
	if (!pMesh.mTextureCoordinates.empty())
	{ // TEXTURE COORDINATES
		unsigned int VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, pMesh.mTextureCoordinates.size() * sizeof(float), &pMesh.mTextureCoordinates.front(), GL_STATIC_DRAW);
		const GLint attributeIndex = static_cast<GLint>(drawInfo.mShaderID.getAttributeLocation("VertexTexCoord"));
		glVertexAttribPointer(attributeIndex, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(attributeIndex);
	}

	LOG_INFO("Mesh '{}' loaded given ID: {}", pMesh.mName, pMesh.mID);
	mMeshManager.insert({pMesh.mID, drawInfo});
}

int OpenGLAPI::getPolygonMode(const DrawCall::DrawMode& pDrawMode)
{
	switch (pDrawMode)
	{
	case DrawCall::DrawMode::Fill: 		return GL_FILL;
	case DrawCall::DrawMode::Wireframe: return GL_LINE;
	default: 					return -1;
	}
}

void OpenGLAPI::setClearColour(const float &pRed, const float &pGreen, const float &pBlue)
{
	mGLADContext->ClearColor(pRed, pGreen, pBlue, 1.0f);
}

void OpenGLAPI::initialiseTextures()
{
	{ // Load all the textures in the textures directory
		std::vector<std::string> textureFileNames = File::getAllFileNames(File::textureDirectory);

		for (size_t i = 0; i < textureFileNames.size(); ++i)
			mTextures.insert({textureFileNames[i], loadTexture(textureFileNames[i])});
	}

	{ // Setup the available texture units. These map the uniform sampler2D slots found in the shader to texture units
		mTextureShader.use();
		for (int i = 0; i < cMaxTextureUnits; ++i)
		{
			std::string textureUniformName = "texture" + std::to_string(i);
			mTextureShader.setUniform(textureUniformName, i);
		}
	}
}

unsigned int OpenGLAPI::loadTexture(const std::string &pFileName)
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
	LOG_INFO("Texture '{}' loaded given ID: {}", pFileName, textureID);

	return textureID;
}

OpenGLAPI::Shader::Shader(const std::string &pName)
	: mName(pName)
	, mSourcePath(File::GLSLShaderDirectory)
{
	load();
}

void OpenGLAPI::Shader::use()
{
	glUseProgram(mHandle);
}

int OpenGLAPI::Shader::getAttributeLocation(const std::string &pName)
{
	const GLint location = glGetAttribLocation(mHandle, pName.c_str());
	ZEPHYR_ASSERT(location != -1, "Failed to find the location of {} in shader {}.", pName, mName);
	return static_cast<int>(location);
}

int OpenGLAPI::Shader::getUniformLocation(const std::string &pName)
{
	int location = glGetUniformLocation(mHandle, pName.c_str());
	// TODO: assert iterate over the loaded shaders to see if the shader ID exists
	ZEPHYR_ASSERT(location != GL_INVALID_VALUE, "ShaderID is not a value generated by OpenGL");
	ZEPHYR_ASSERT(location != GL_INVALID_OPERATION, "ShaderID is not a program object or has not been successfully linked");
	ZEPHYR_ASSERT(location != -1, "UniformName does not correspond to an active uniform variable in ShaderID or UniformName starts with the reserved prefix 'gl_'");

	return location;
}

void OpenGLAPI::Shader::setUniform(const std::string &pName, const bool &pValue)
{
	glUniform1i(getUniformLocation(pName), (int)pValue); // Setting a boolean is treated as integer for gl shaders
}

void OpenGLAPI::Shader::setUniform(const std::string &pName, const int &pValue)
{
	glUniform1i(getUniformLocation(pName), (GLint)pValue);
}

void OpenGLAPI::Shader::setUniform(const std::string &pName, const glm::mat4 &pValue)
{
	glUniformMatrix4fv(getUniformLocation(pName), 1, GL_FALSE, glm::value_ptr(pValue));
}

bool OpenGLAPI::Shader::hasCompileErrors(const Type& pType, const unsigned int pID)
{
	int success;
	if (pType == Type::Program)
	{
		glGetProgramiv(pID, GL_LINK_STATUS, &success);
		if (!success)
		{
			char infoLog[1024];
			glGetProgramInfoLog(pID, 1024, NULL, infoLog);
			LOG_ERROR("Program linking failed with info: {}", infoLog);
			return true;
		}
	}
	else
	{
		glGetShaderiv(pID, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			char infoLog[1024];
			glGetShaderInfoLog(pID, 1024, NULL, infoLog);
			LOG_ERROR("Shader compilation failed with info: {}", infoLog);
			return true;
		}
	}

	return false;
}

void OpenGLAPI::Shader::load()
{
	const std::string vertexShaderPath 		= mSourcePath + mName + ".vert";
	const std::string fragmentShaderPath 	= mSourcePath + mName + ".frag";
	ZEPHYR_ASSERT(File::exists(vertexShaderPath), "Vertex shader does not exist at path {}", vertexShaderPath);
	ZEPHYR_ASSERT(File::exists(fragmentShaderPath), "Fragment shader does not exist at path {}", fragmentShaderPath);

	unsigned int vertexShader;
	{
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		std::string source = File::readFromFile(vertexShaderPath);
		const char *vertexShaderSource = source.c_str();
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);
		ZEPHYR_ASSERT(!hasCompileErrors(Type::Vertex, vertexShader), "Failed to compile vertex shader {}", mName + ".vert")
	}

	unsigned int fragmentShader;
	{
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		std::string source = File::readFromFile(fragmentShaderPath);
		const char *fragmentShaderSource = source.c_str();
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);
		ZEPHYR_ASSERT(!hasCompileErrors(Type::Fragment, fragmentShader), "Failed to compile fragment shader {}", mName + ".frag")
	}

	{
		mHandle = glCreateProgram();
		glAttachShader(mHandle, vertexShader);
		glAttachShader(mHandle, fragmentShader);
		glLinkProgram(mHandle);
		ZEPHYR_ASSERT(!hasCompileErrors(Type::Program, mHandle), "Failed to link shader {}", mName)
	}

	// Delete the shaders after linking as they're no longer needed
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	LOG_INFO("Shader '{}' loaded given ID: {}", mName, mHandle);
}

OpenGLAPI::DrawInfo::DrawInfo(Shader &pShader)
	: mShaderID(pShader)
	, mVAO(invalidHandle)
	, mVBO(invalidHandle)
	, mEBO(invalidHandle)
	, mDrawMode(invalidHandle)
	, mDrawSize(invalidHandle)
	, mDrawMethod(DrawMethod::Null)
{}

GladGLContext* OpenGLAPI::initialiseGLAD()
{
	GladGLContext* GLADContext = (GladGLContext *)malloc(sizeof(GladGLContext));
	int version = gladLoadGLContext(GLADContext, glfwGetProcAddress);
	ZEPHYR_ASSERT(GLADContext && version != 0, "Failed to initialise GLAD GL context")
	// TODO: Add an assert here for GLAD_VERSION to equal to cOpenGLVersion
	LOG_INFO("Initialised GLAD using OpenGL {}.{}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
	return GLADContext;
}

void OpenGLAPI::windowSizeCallback(GLFWwindow* pWindow, int pWidth, int pHeight)
{
	LOG_INFO("Window resolution changed to {}x{}", pWidth, pHeight);
	glViewport(0, 0, pWidth, pHeight);
	OpenGLWindow::currentWindow->onResize(pWidth, pHeight);
}