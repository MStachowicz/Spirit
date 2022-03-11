#include "OpenGLAPI.hpp"

#include "glad/gl.h"
#include "GLFW/glfw3.h" // Used to initialise GLAD using glfwGetProcAddress

#include "DrawCall.hpp"
#include "LightManager.hpp"

#include "glm/ext/matrix_transform.hpp" // perspective, translate, rotate
#include "glm/gtc/type_ptr.hpp"

#include "imgui.h"

OpenGLAPI::OpenGLAPI(const MeshManager& pMeshManager, const TextureManager& pTextureManager, const LightManager& pLightManager)
	: GraphicsAPI(pMeshManager, pTextureManager, pLightManager)
	, cOpenGLVersionMajor(3)
	, cOpenGLVersionMinor(3)
	, mWindow(cOpenGLVersionMajor, cOpenGLVersionMinor)
	, mGLADContext(initialiseGLAD())
	, mWindowClearColour{0.0f, 0.0f, 0.0f}
{
	mShaders = {Shader("texture"), Shader("material"), Shader("colour"), Shader("uniformColour")};
	mTextureShaderIndex = 0;
	mMaterialShaderIndex = 1;
	mUniformShaderIndex = 3;

	mMeshManager.ForEach([this](const auto &mesh) { initialiseMesh(mesh); }); // Depends on mShaders being initialised.
	mTextureManager.ForEach([this](const auto &texture) { initialiseTexture(texture); });

    glfwSetWindowSizeCallback(mWindow.mHandle, windowSizeCallback);
	glViewport(0, 0, mWindow.mWidth, mWindow.mHeight);
	glEnable(GL_DEPTH_TEST);

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

void OpenGLAPI::onFrameStart()
{
	clearBuffers();
	mWindow.startImGuiFrame();

	if (ImGui::Begin("OpenGL options"))
	{
		if (ImGui::ColorEdit3("Window clear colour", mWindowClearColour))
			setClearColour(mWindowClearColour[0], mWindowClearColour[1], mWindowClearColour[2]);
	}
	ImGui::End();

	{ // Set all the light uniforms ready for draw() calls
		mShaders[mMaterialShaderIndex].use();
		mShaders[mMaterialShaderIndex].setUniform("viewPosition", mViewPosition);

		mLightManager.getPointLights().ForEach([&](const PointLight &pointLight)
		{
			glm::vec3 diffuseColour = pointLight.mColour * pointLight.mDiffuse;
			glm::vec3 ambientColour = diffuseColour * pointLight.mAmbient;

			mShaders[mMaterialShaderIndex].setUniform("light.ambient", ambientColour);
			mShaders[mMaterialShaderIndex].setUniform("light.diffuse", diffuseColour);
			mShaders[mMaterialShaderIndex].setUniform("light.specular", pointLight.mSpecular);
			mShaders[mMaterialShaderIndex].setUniform("light.position", pointLight.mPosition);
		});
	}
}


void OpenGLAPI::draw(const DrawCall& pDrawCall)
{
	const DrawInfo& drawInfo = getDrawInfo(pDrawCall.mMesh); // Grab the DrawInfo for the Mesh requested.
	const Shader* shader = nullptr;

	switch (pDrawCall.mDrawStyle)
	{
	case DrawStyle::Default: // Default renders objects with texture
	case DrawStyle::Textured:
		shader = &mShaders[mTextureShaderIndex];
		shader->use();

		ZEPHYR_ASSERT(shader->getTexturesUnitsCount() > 0, "Shader selected for textured draw does not have any texture units.");

		if (pDrawCall.mTexture.has_value())
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, getTextureHandle(pDrawCall.mTexture.value()));
		}
		else
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, getTextureHandle(mTextureManager.getTextureID("missing")));
		}
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, 0);
		break;
	case DrawStyle::UniformColour:
		shader = &mShaders[mUniformShaderIndex];
		break;
	default:
		break;
	}
	shader->use();

	glm::mat4 trans = glm::translate(glm::mat4(1.0f), pDrawCall.mPosition);
	trans = glm::rotate(trans, glm::radians(pDrawCall.mRotation.x), glm::vec3(1.0, 0.0, 0.0));
	trans = glm::rotate(trans, glm::radians(pDrawCall.mRotation.y), glm::vec3(0.0, 1.0, 0.0));
	trans = glm::rotate(trans, glm::radians(pDrawCall.mRotation.z), glm::vec3(0.0, 0.0, 1.0));
	trans = glm::scale(trans, pDrawCall.mScale);

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

	shader->setUniform("model", trans);
	shader->setUniform("view", mViewMatrix);
	shader->setUniform("projection", projection);

	glPolygonMode(GL_FRONT_AND_BACK, getPolygonMode(pDrawCall.mDrawMode));
	getVAO(pDrawCall.mMesh).bind();

	if (drawInfo.mDrawMethod == DrawInfo::DrawMethod::Indices)
		glDrawElements(drawInfo.mDrawMode, static_cast<GLsizei>(drawInfo.mDrawSize), GL_UNSIGNED_INT, 0);
	else if (drawInfo.mDrawMethod == DrawInfo::DrawMethod::Array)
		glDrawArrays(drawInfo.mDrawMode, 0, static_cast<GLsizei>(drawInfo.mDrawSize));
}

void OpenGLAPI::postDraw()
{
	mWindow.renderImGui();
	mWindow.swapBuffers();
}

bool OpenGLAPI::isMeshValidForShader(const Mesh& pMesh, const Shader& pShader)
{
	const auto attributes = pShader.getRequiredAttributes();

	for (const auto& attribute : attributes)
	{
		switch (attribute)
		{
		case Shader::Attribute::Position3D:
			if(pMesh.mVertices.empty())
				return false;
			break;
		case Shader::Attribute::Normal3D:
			if(pMesh.mNormals.empty())
				return false;
			break;
		case Shader::Attribute::ColourRGB:
			if(pMesh.mColours.empty())
				return false;
			break;
		case Shader::Attribute::TextureCoordinate2D:
			if(pMesh.mTextureCoordinates.empty())
				return false;
			break;
		default:
			ZEPHYR_ASSERT(false, "Missing Mesh attribute check for Shader::Attribute {}", attribute);
			return false;
		}
	}

	return true;
}

const OpenGLAPI::DrawInfo& OpenGLAPI::getDrawInfo(const MeshID& pMeshID)
{
	const auto it = mDrawInfos.find(pMeshID);
	ZEPHYR_ASSERT(it != mDrawInfos.end(), "No draw info found for this Mesh ID. Was the mesh correctly initialised?");
	return it->second;
}

const OpenGLAPI::VAO& OpenGLAPI::getVAO(const MeshID& pMeshID)
{
	const auto it = mVAOs.find(pMeshID);
	ZEPHYR_ASSERT(it != mVAOs.end(), "No VAO found for this Mesh ID. Was the mesh correctly initialised?", pMeshID)
	return it->second;
}

const OpenGLAPI::TextureHandle& OpenGLAPI::getTextureHandle(const TextureID& pTextureID)
{
	const auto it = mTextures.find(pTextureID);
	ZEPHYR_ASSERT(it != mTextures.end(), "No Texture handle found for this Mesh ID. Was the mesh correctly initialised?", pMeshID)
	return it->second;
}

OpenGLAPI::VAO::VAO()
: mHandle(0)
{
	glGenVertexArrays(1, &mHandle);
}
void OpenGLAPI::VAO::bind() const
{
	glBindVertexArray(mHandle);
}
void OpenGLAPI::VAO::release()
{
	//throw std::logic_error( "Not allowed to delete yet" );
	glDeleteVertexArrays(1, &mHandle);
}
void OpenGLAPI::VBO::release()
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
std::optional<OpenGLAPI::VBO> OpenGLAPI::bufferAttributeData(const std::vector<T>& pData, const Shader::Attribute& pAttribute)
{
	if (!pData.empty())
	{
		unsigned int VBOHandle = 0;
		glGenBuffers(1, &VBOHandle);
		glBindBuffer(GL_ARRAY_BUFFER, VBOHandle);
		glBufferData(GL_ARRAY_BUFFER, pData.size() * sizeof(T), &pData.front(), GL_STATIC_DRAW);
		const GLint attributeIndex = static_cast<GLint>(Shader::getAttributeLocation(pAttribute));
		const GLint attributeComponentCount = static_cast<GLint>(Shader::getAttributeComponentCount(pAttribute));
		glVertexAttribPointer(attributeIndex, attributeComponentCount, getGLFWType<T>(), GL_FALSE, attributeComponentCount * sizeof(T), (void *)0);
		glEnableVertexAttribArray(attributeIndex);
		return VBO(VBOHandle);
	}
	else
		return std::nullopt;
}

int OpenGLAPI::getPolygonMode(const DrawMode& pDrawMode)
{
	switch (pDrawMode)
	{
	case DrawMode::Fill: 		return GL_FILL;
	case DrawMode::Wireframe: 	return GL_LINE;
	default:
   		ZEPHYR_ASSERT(false, "DrawMode does not have a conversion to GLFW type.");
		return -1;
	}
}

void OpenGLAPI::setClearColour(const float &pRed, const float &pGreen, const float &pBlue)
{
	mGLADContext->ClearColor(pRed, pGreen, pBlue, 1.0f);
}

void OpenGLAPI::initialiseMesh(const Mesh& pMesh)
{
	DrawInfo drawInfo = DrawInfo();

	for (const auto& shader : mShaders)
		if (isMeshValidForShader(pMesh, shader))
			drawInfo.mShadersAvailable.push_back(&shader);
	ZEPHYR_ASSERT(!drawInfo.mShadersAvailable.empty(), "Shaders available cannot be empty. Mesh needs at least one shader to draw with.")

	drawInfo.mDrawMode 			= GL_TRIANGLES;	// OpenGLAPI only supports GL_TRIANGLES at this revision
	drawInfo.mDrawMethod 		= pMesh.mIndices.empty() ? DrawInfo::DrawMethod::Array : DrawInfo::DrawMethod::Indices;
	drawInfo.mDrawSize 			= pMesh.mIndices.empty() ? static_cast<int>(pMesh.mVertices.size()) : static_cast<int>(pMesh.mIndices.size());
	mDrawInfos.emplace(std::make_pair(pMesh.mID, drawInfo));

	ZEPHYR_ASSERT(!pMesh.mVertices.empty(), "Cannot set a mesh handle for a mesh with no position data.")
	if (!pMesh.mColours.empty())
		ZEPHYR_ASSERT(pMesh.mColours.size() == pMesh.mVertices.size(), ("Size of colour data ({}) does not match size of position data ({}), cannot buffer the colour data", pMesh.mColours.size(), pMesh.mVertices.size()));

	const auto pair = mVAOs.emplace(std::make_pair(pMesh.mID, VAO()));
	getVAO(pMesh.mID).bind(); // Bind VAO first as following VBOs will be assigned to this VAO.

	if (!pMesh.mIndices.empty())
	{ // INDICES (Element buffer - re-using data)
		unsigned int EBO;
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, pMesh.mIndices.size() * sizeof(int), &pMesh.mIndices.front(), GL_STATIC_DRAW);
	}
	ZEPHYR_ASSERT(mVBOs.find(pMesh.mID) == mVBOs.end(), "VBO data for this mesh already exists. Release the data before re-initialising");

	mVBOs.emplace(std::make_pair(pMesh.mID,
	std::array<std::optional<VBO>, util::toIndex(Shader::Attribute::Count)>
	{
		bufferAttributeData<float>(pMesh.mVertices, Shader::Attribute::Position3D)
		, bufferAttributeData<float>(pMesh.mNormals, Shader::Attribute::Normal3D)
		, bufferAttributeData<float>(pMesh.mColours, Shader::Attribute::ColourRGB)
		, bufferAttributeData<float>(pMesh.mTextureCoordinates, Shader::Attribute::TextureCoordinate2D)
	}));
	LOG_INFO("Mesh '{}' loaded given ID: {}", pMesh.mName, pMesh.mID);
}

void OpenGLAPI::initialiseTexture(const Texture& pTexture)
{
	ZEPHYR_ASSERT(mTextures.find(pTexture.mID) == mTextures.end(), "Data for this texture already exists. Release the data before re-initialising");

	unsigned int textureHandle;
	glGenTextures(1, &textureHandle);
	glBindTexture(GL_TEXTURE_2D, textureHandle);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	const int channelType = pTexture.mNumberOfChannels == 4 ? GL_RGBA : GL_RGB;
	glTexImage2D(GL_TEXTURE_2D, 0, channelType, pTexture.mWidth, pTexture.mHeight, 0, channelType, GL_UNSIGNED_BYTE, pTexture.getData());
	glGenerateMipmap(GL_TEXTURE_2D);
	ZEPHYR_ASSERT(textureHandle != -1, "Texture {} failed to load", pTexture.mName);

	mTextures.emplace(std::make_pair(pTexture.mID, textureHandle));

	LOG_INFO("Texture '{}' loaded given ID: {}", pTexture.mName, textureHandle);
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