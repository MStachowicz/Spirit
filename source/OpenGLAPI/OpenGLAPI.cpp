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
	, mWindowClearColour{0.0f, 0.0f, 0.0f}
	, mDepthTest(true)
	, mZNearPlane(0.1f)
	, mZFarPlane (100.0f)
	, mFOV(45.f)
	, mWindow(cOpenGLVersionMajor, cOpenGLVersionMinor)
	, mGLADContext(initialiseGLAD())
	, mTexture1ShaderIndex(0)
	, mTexture2ShaderIndex(1)
	, mMaterialShaderIndex(2)
	, mUniformShaderIndex(4)
	, mLightMapIndex(5)
	, mShaders{ Shader("texture1"), Shader("texture2"), Shader("material"), Shader("colour"), Shader("uniformColour"), Shader("lightMap") }
{
	mMeshManager.ForEach([this](const auto &mesh) { initialiseMesh(mesh); }); // Depends on mShaders being initialised.
	mTextureManager.ForEach([this](const auto &texture) { initialiseTexture(texture); });

    glfwSetWindowSizeCallback(mWindow.mHandle, windowSizeCallback);
	glViewport(0, 0, mWindow.mWidth, mWindow.mHeight);

	setDepthTest(mDepthTest);

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

void OpenGLAPI::setDepthTest(const bool& pDepthTest)
{
	if (pDepthTest)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
}

void OpenGLAPI::clearBuffers()
{
	mGLADContext->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLAPI::onFrameStart()
{
	clearBuffers();
	mWindow.startImGuiFrame();

	if (ImGui::Begin("OpenGL options", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("OpenGL version: {}.{}", cOpenGLVersionMajor, cOpenGLVersionMinor);
		if (ImGui::ColorEdit3("Window clear colour", mWindowClearColour))
			setClearColour(mWindowClearColour[0], mWindowClearColour[1], mWindowClearColour[2]);
		if (ImGui::Checkbox("Depth test", &mDepthTest))
			setDepthTest(mDepthTest);

		ImGui::SliderFloat("Field of view", &mFOV, 1.f, 120.f);
		ImGui::SliderFloat("Z near plane", &mZNearPlane, 0.001f, 15.f);
		ImGui::SliderFloat("Z far plane", &mZFarPlane, 15.f, 300.f);
		ImGui::Text(("Viewport size: " + std::to_string(mWindow.mWidth) + "x" + std::to_string(mWindow.mHeight)).c_str());
		ImGui::Text(("View position: " + std::to_string(mViewPosition.x) + "," + std::to_string(mViewPosition.y) + "," + std::to_string(mViewPosition.z)).c_str());
	}
	ImGui::End();

	mProjection = glm::perspective(glm::radians(mFOV), mWindow.mAspectRatio, mZNearPlane, mZFarPlane);

	{ // Set all light uniforms for material shader
		mShaders[mMaterialShaderIndex].use();
		mShaders[mMaterialShaderIndex].setUniform("viewPosition", mViewPosition);
		mLightManager.getPointLights().ForEach([&](const PointLight &pointLight)
		{
			const glm::vec3 diffuseColour = pointLight.mColour  * pointLight.mDiffuseIntensity;
        	const glm::vec3 ambientColour = diffuseColour * pointLight.mAmbientIntensity;
        	mShaders[mMaterialShaderIndex].setUniform("light.ambient", ambientColour);
        	mShaders[mMaterialShaderIndex].setUniform("light.diffuse", diffuseColour);
        	mShaders[mMaterialShaderIndex].setUniform("light.specular", glm::vec3(pointLight.mSpecularIntensity));
			mShaders[mMaterialShaderIndex].setUniform("light.position", pointLight.mPosition);
		});
	}
	{// Set all light uniforms for light map shader
		mShaders[mLightMapIndex].use();
		mShaders[mLightMapIndex].setUniform("viewPosition", mViewPosition);

		size_t count = 0;
		ZEPHYR_ASSERT(mLightManager.getPointLights().size() == 4, "Only an exact number of 4 pointlights is supported.")
		mLightManager.getPointLights().ForEach([&](const PointLight& pointLight)
		{
			const std::string uniform = "pointLights[" + std::to_string(count) + "]";
			const glm::vec3 diffuseColour = pointLight.mColour  * pointLight.mDiffuseIntensity;
        	const glm::vec3 ambientColour = diffuseColour * pointLight.mAmbientIntensity;

			mShaders[mLightMapIndex].setUniform((uniform + ".position").c_str(), pointLight.mPosition);
			mShaders[mLightMapIndex].setUniform((uniform + ".ambient").c_str(), ambientColour);
			mShaders[mLightMapIndex].setUniform((uniform + ".diffuse").c_str(), diffuseColour);
			mShaders[mLightMapIndex].setUniform((uniform + ".specular").c_str(), glm::vec3(pointLight.mSpecularIntensity));
			mShaders[mLightMapIndex].setUniform((uniform + ".constant").c_str(), pointLight.mConstant);
			mShaders[mLightMapIndex].setUniform((uniform + ".linear").c_str(), pointLight.mLinear);
			mShaders[mLightMapIndex].setUniform((uniform + ".quadratic").c_str(), pointLight.mQuadratic);
			count++;
		});

		ZEPHYR_ASSERT(mLightManager.getDirectionalLights().size() <= 1, "Only one directional light is supported by OpenGLAPI")
		count = 0;
		mLightManager.getDirectionalLights().ForEach([&](const DirectionalLight& directionalLight)
		{
			const glm::vec3 diffuseColour = directionalLight.mColour  * directionalLight.mDiffuseIntensity;
        	const glm::vec3 ambientColour = diffuseColour * directionalLight.mAmbientIntensity;
			mShaders[mLightMapIndex].setUniform("dirLight.direction", directionalLight.mDirection);
			mShaders[mLightMapIndex].setUniform("dirLight.ambient", ambientColour);
			mShaders[mLightMapIndex].setUniform("dirLight.diffuse", diffuseColour);
			mShaders[mLightMapIndex].setUniform("dirLight.specular", glm::vec3(directionalLight.mSpecularIntensity));
			count++;
		});

		ZEPHYR_ASSERT(mLightManager.getSpotlightsLights().size() <= 1, "Only one spotlight light is supported by OpenGLAPI")
		count = 0;
		mLightManager.getSpotlightsLights().ForEach([&](const SpotLight& light)
		{
			const glm::vec3 diffuseColour = light.mColour  * light.mDiffuseIntensity;
        	const glm::vec3 ambientColour = diffuseColour * light.mAmbientIntensity;
        	mShaders[mLightMapIndex].setUniform("spotLight.position", light.mPosition);
        	mShaders[mLightMapIndex].setUniform("spotLight.direction", light.mDirection);
        	mShaders[mLightMapIndex].setUniform("spotLight.diffuse", diffuseColour);
        	mShaders[mLightMapIndex].setUniform("spotLight.ambient", ambientColour);
        	mShaders[mLightMapIndex].setUniform("spotLight.specular", glm::vec3(light.mSpecularIntensity));
        	mShaders[mLightMapIndex].setUniform("spotLight.constant", light.mConstant);
        	mShaders[mLightMapIndex].setUniform("spotLight.linear", light.mLinear);
        	mShaders[mLightMapIndex].setUniform("spotLight.quadratic", light.mQuadratic);
        	mShaders[mLightMapIndex].setUniform("spotLight.cutOff", light.mCutOff);
        	mShaders[mLightMapIndex].setUniform("spotLight.cutOff", light.mOuterCutOff);
			count++;
		});
	}
}

void OpenGLAPI::draw(const DrawCall& pDrawCall)
{
	const DrawInfo& drawInfo = getDrawInfo(pDrawCall.mMesh); // Grab the DrawInfo for the Mesh requested.
	const Shader* shader = nullptr;

	switch (pDrawCall.mDrawStyle)
	{
	case DrawStyle::Textured:
		if(pDrawCall.mTexture1.has_value() && pDrawCall.mTexture2.has_value())
		{
			shader = &mShaders[mTexture2ShaderIndex];
			shader->use();
			shader->setUniform("mixFactor", pDrawCall.mMixFactor.value());
		}
		else
		{
			shader = &mShaders[mTexture1ShaderIndex];
			shader->use();
		}
		ZEPHYR_ASSERT(shader->getTexturesUnitsCount() > 0, "Shader selected for textured draw does not have any texture units.");

		if (pDrawCall.mTexture1.has_value())
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, getTextureHandle(pDrawCall.mTexture1.value()));
		}
		else
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, getTextureHandle(mTextureManager.getTextureID("missing")));
		}
		if (pDrawCall.mTexture2.has_value())
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, getTextureHandle(pDrawCall.mTexture2.value()));
		}
		else
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, getTextureHandle(mTextureManager.getTextureID("missing")));
		}
		break;
	case DrawStyle::UniformColour:
		shader = &mShaders[mUniformShaderIndex];
		shader->use();
		shader->setUniform("colour", pDrawCall.mColour.value());
		break;
	case DrawStyle::Material:
		shader = &mShaders[mMaterialShaderIndex];
		shader->use();
		shader->setUniform("material.ambient", pDrawCall.mMaterial.value().ambient);
       	shader->setUniform("material.diffuse", pDrawCall.mMaterial.value().diffuse);
        shader->setUniform("material.specular", pDrawCall.mMaterial.value().specular);
        shader->setUniform("material.shininess", pDrawCall.mMaterial.value().shininess);
		break;
	case DrawStyle::LightMap:
		shader = &mShaders[mLightMapIndex];
		shader->use();

		glActiveTexture(GL_TEXTURE0);
		if (pDrawCall.mDiffuseTextureID.has_value())
			glBindTexture(GL_TEXTURE_2D, getTextureHandle(pDrawCall.mDiffuseTextureID.value()));
		else
			glBindTexture(GL_TEXTURE_2D, getTextureHandle(mTextureManager.getTextureID("missing")));

		glActiveTexture(GL_TEXTURE1);
		if (pDrawCall.mSpecularTextureID.has_value())
			glBindTexture(GL_TEXTURE_2D, getTextureHandle(pDrawCall.mSpecularTextureID.value()));
		else
			glBindTexture(GL_TEXTURE_2D, getTextureHandle(mTextureManager.getTextureID("missing")));
			shader->setUniform("lightMap.shininess", pDrawCall.mShininess.value());
		break;
	default:
		break;
	}
	ZEPHYR_ASSERT(shader != nullptr, "Shader to draw with has not been set.")

	glm::mat4 trans = glm::translate(glm::mat4(1.0f), pDrawCall.mPosition);
	trans = glm::rotate(trans, glm::radians(pDrawCall.mRotation.x), glm::vec3(1.0, 0.0, 0.0));
	trans = glm::rotate(trans, glm::radians(pDrawCall.mRotation.y), glm::vec3(0.0, 1.0, 0.0));
	trans = glm::rotate(trans, glm::radians(pDrawCall.mRotation.z), glm::vec3(0.0, 0.0, 1.0));
	trans = glm::scale(trans, pDrawCall.mScale);

	{// Optimisation: Only set these once per shader in onFrameStart
		shader->setUniform("model", trans);
		shader->setUniform("view", mViewMatrix);
		shader->setUniform("projection", mProjection);
	}

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

	const float width = static_cast<float>(pWidth);
	const float height = static_cast<float>(pHeight);

	ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2(width, height);
	io.FontGlobalScale = ((width * height) / 1.4f ) / (1920.f * 1080.f);;

	OpenGLWindow::currentWindow->mWidth = pWidth;
	OpenGLWindow::currentWindow->mHeight = pHeight;
	OpenGLWindow::currentWindow->mAspectRatio = width / height;
}