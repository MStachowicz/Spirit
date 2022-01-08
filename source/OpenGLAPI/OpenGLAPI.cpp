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
	, mMaterialShader("material")
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
		const DrawInfo& drawInfo = mDataManager.getDrawInfo(mDrawQueue[i].mMesh);
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
		mDataManager.bindVAO(mDrawQueue[i].mMesh);

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


void OpenGLAPI::GPUDataManager::assignVBOs(const Mesh& pMesh, const Shader& pShader)
{
	bindVAO(pMesh.mID); // Bind VAO first as following VBOs will be assigned to this VAO.
	getDrawInfo(pMesh.mID).mShaderID.use();

	// Per vertex attributes
	unsigned int positionsVBO = bufferAttributeData<float>(pMesh.mVertices, Shader::Attribute::Position3D, pShader);
	// Remaining data is optional:
	unsigned int normalsVBO = bufferAttributeData<glm::vec3>(pMesh.mNormals, Shader::Attribute::Normal3D, pShader);
	unsigned int coloursVBO = bufferAttributeData<float>(pMesh.mColours, Shader::Attribute::ColourRGB, pShader);
	unsigned int textureCoordinatesVBO = bufferAttributeData<float>(pMesh.mTextureCoordinates, Shader::Attribute::TextureCoordinate2D, pShader);

	const auto it = mVBOs.find(pMesh.mID);
	if (it != mVBOs.end())
	{
		// If this VBO was previously created it wll be replaced and the old VBO is freed from GPU memory.
		it->second[Shader::toIndex(Shader::Attribute::Position3D)] 			= positionsVBO != 0 ? std::make_unique<VBO>(positionsVBO) : nullptr;
		it->second[Shader::toIndex(Shader::Attribute::Normal3D)] 			= normalsVBO != 0 ? std::make_unique<VBO>(normalsVBO) : nullptr;
		it->second[Shader::toIndex(Shader::Attribute::ColourRGB)] 			= coloursVBO != 0 ? std::make_unique<VBO>(coloursVBO) : nullptr;
		it->second[Shader::toIndex(Shader::Attribute::TextureCoordinate2D)] = textureCoordinatesVBO != 0 ? std::make_unique<VBO>(textureCoordinatesVBO) : nullptr;
	}
	else
	{
		// Creating a temporary array to be able to index by pAttribute into the storage.
		std::array<std::unique_ptr<VBO>, Shader::toIndex(Shader::Attribute::Count)> toMove;
		toMove[Shader::toIndex(Shader::Attribute::Position3D)] 			= positionsVBO != 0 ? std::make_unique<VBO>(positionsVBO) : nullptr;
		toMove[Shader::toIndex(Shader::Attribute::Normal3D)] 			= normalsVBO != 0 ? std::make_unique<VBO>(normalsVBO) : nullptr;
		toMove[Shader::toIndex(Shader::Attribute::ColourRGB)] 			= coloursVBO != 0 ? std::make_unique<VBO>(coloursVBO) : nullptr;
		toMove[Shader::toIndex(Shader::Attribute::TextureCoordinate2D)] = textureCoordinatesVBO != 0 ? std::make_unique<VBO>(textureCoordinatesVBO) : nullptr;

		mVBOs.emplace(std::make_pair(pMesh.mID, std::move(toMove)));
	}
}

void OpenGLAPI::GPUDataManager::loadMesh(const Mesh& pMesh, const Shader& pShader)
{
	ZEPHYR_ASSERT(!pMesh.mVertices.empty(), "Cannot set a mesh handle for a mesh with no position data.")
	if (!pMesh.mColours.empty())
		ZEPHYR_ASSERT(pMesh.mColours.size() == pMesh.mVertices.size(), ("Size of colour data ({}) does not match size of position data ({}), cannot buffer the colour data", pMesh.mColours.size(), pMesh.mVertices.size()));

	assignVAO(pMesh.mID);
	bindVAO(pMesh.mID);

	DrawInfo drawInfo 		= DrawInfo(pShader);
	drawInfo.mDrawMode 		= GL_TRIANGLES;	// OpenGLAPI only supports GL_TRIANGLES at this revision
	drawInfo.mDrawMethod 	= pMesh.mIndices.empty() ? DrawInfo::DrawMethod::Array : DrawInfo::DrawMethod::Indices;
	drawInfo.mDrawSize 		= pMesh.mIndices.empty() ? static_cast<int>(pMesh.mVertices.size()) : static_cast<int>(pMesh.mIndices.size());
	drawInfo.mShaderID.use();

	assignDrawInfo(pMesh.mID, drawInfo);

	if (!pMesh.mIndices.empty())
	{ // INDICES (Element buffer - re-using data)
		unsigned int EBO;
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, pMesh.mIndices.size() * sizeof(int), &pMesh.mIndices.front(), GL_STATIC_DRAW);
	}

	assignVBOs(pMesh, drawInfo.mShaderID);
}

void OpenGLAPI::initialiseMesh(const Mesh &pMesh)
{
	mDataManager.loadMesh(pMesh, !pMesh.mNormals.empty() ? mMaterialShader : mTextureShader);
	LOG_INFO("Mesh '{}' loaded given ID: {}", pMesh.mName, pMesh.mID);
}

const OpenGLAPI::DrawInfo& OpenGLAPI::GPUDataManager::getDrawInfo(const MeshID& pMeshID)
{
	const auto it = mDrawInfos.find(pMeshID);
	ZEPHYR_ASSERT(it != mDrawInfos.end(), "No draw info found for this Mesh ID. Was the mesh correctly initialised?");
	return it->second;
}

OpenGLAPI::DrawInfo::DrawInfo(const Shader& pShader)
	: mShaderID(pShader)
	, mEBO(invalidHandle)
	, mDrawMode(invalidHandle)
	, mDrawSize(invalidHandle)
	, mDrawMethod(DrawMethod::Null)
{}

void OpenGLAPI::GPUDataManager::assignVAO(const MeshID& pMeshID)
{
	mVAOs.emplace(std::make_pair(pMeshID, std::move(std::make_unique<VAO>())));
}

void OpenGLAPI::GPUDataManager::bindVAO(const MeshID& pMeshID)
{
	const auto it = mVAOs.find(pMeshID);
	ZEPHYR_ASSERT(it != mVAOs.end(), "Trying to bind a VAO that doesnt exist. Iniitalise this mesh before calling bindVAO.", pMeshID)
	it->second->bind();
}

void OpenGLAPI::GPUDataManager::assignDrawInfo(const MeshID& pMeshID, const DrawInfo& pDrawInfo)
{
	mDrawInfos.emplace(std::make_pair(pMeshID, pDrawInfo));
}

void OpenGLAPI::GPUDataManager::VAO::bind() const
{
	glBindVertexArray(mHandle);
}

OpenGLAPI::GPUDataManager::VAO::VAO()
: mHandle(0)
{
	glGenVertexArrays(1, &mHandle);
}

OpenGLAPI::GPUDataManager::VAO::~VAO()
{
	//throw std::logic_error( "Not allowed to delete yet" );
	glDeleteVertexArrays(1, &mHandle);
}
OpenGLAPI::GPUDataManager::VBO::~VBO()
{
	//throw std::logic_error( "Not allowed to delete yet" );
	glDeleteBuffers(1, &mHandle);
}


template<class T>
int getGLFWType()
{
	if (constexpr(std::is_same_v<T, int>))
		return GL_INT;
	else if (constexpr(std::is_same_v<T, float>))
		return GL_FLOAT;
	else if (constexpr(std::is_same_v<T, glm::vec3>))
		return GL_FLOAT;
	else
	{
		ZEPHYR_ASSERT(false, "Could not convert the template type to a GLFW type.")
		return -1;
	}
};

template <class T>
unsigned int OpenGLAPI::GPUDataManager::bufferAttributeData(const std::vector<T>& pData, const Shader::Attribute& pAttribute, const Shader& pShader)
{
	unsigned int VBOHandle = 0;

	if (!pData.empty())
	{
		glGenBuffers(1, &VBOHandle);
		glBindBuffer(GL_ARRAY_BUFFER, VBOHandle);
		glBufferData(GL_ARRAY_BUFFER, pData.size() * sizeof(T), &pData.front(), GL_STATIC_DRAW);
		const GLint attributeIndex = static_cast<GLint>(pShader.getAttributeLocation(pAttribute));
		const GLint attributeComponentCount = static_cast<GLint>(pShader.getAttributeComponentCount(pAttribute));
		glVertexAttribPointer(attributeIndex, attributeComponentCount, getGLFWType<T>(), GL_FALSE, attributeComponentCount * sizeof(T), (void *)0);
		glEnableVertexAttribArray(attributeIndex);
	}

	return VBOHandle;
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