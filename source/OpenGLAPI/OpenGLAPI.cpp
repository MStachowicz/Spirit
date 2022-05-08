#include "OpenGLAPI.hpp"

#include "glad/gl.h"
#include "GLFW/glfw3.h" // Used to initialise GLAD using glfwGetProcAddress

#include "DrawCall.hpp"
#include "Logger.hpp"

#include "Mesh.hpp"
#include "Texture.hpp"
#include "Light.hpp"

#include "glm/ext/matrix_transform.hpp" // perspective, translate, rotate
#include "glm/gtc/type_ptr.hpp"

#include "imgui.h"

OpenGLAPI::OpenGLAPI(const LightManager& pLightManager)
	: GraphicsAPI(pLightManager)
	, cOpenGLVersionMajor(3)
	, cOpenGLVersionMinor(3)
	, mLinearDepthView(false)
	, mZNearPlane(0.1f)
	, mZFarPlane (100.0f)
	, mFOV(45.f)
	, mWindow(cOpenGLVersionMajor, cOpenGLVersionMinor)
	, mGLADContext(initialiseGLAD()) // TODO: This should only happen on first OpenGLAPI construction (OpenGLInstances.size() == 1)
	, mGLState()
	, mTexture1ShaderIndex(0)
	, mTexture2ShaderIndex(1)
	, mMaterialShaderIndex(2)
	, mUniformShaderIndex(4)
	, mLightMapIndex(5)
	, mDepthViewerIndex(6)
	, mScreenTextureIndex(7)
	, mScreenQuad(0)
	, mMissingTextureID(0)
	, pointLightDrawCount(0)
	, spotLightDrawCount(0)
	, directionalLightDrawCount(0)
	, mBufferDrawType(GLType::BufferDrawType::Colour)
	, mPostProcessingOptions()
	, mShaders{ Shader("texture1"), Shader("texture2"), Shader("material"), Shader("colour"), Shader("uniformColour"), Shader("lightMap"), Shader("depthView"), Shader("screenTexture") }
{
    glfwSetWindowSizeCallback(mWindow.mHandle, windowSizeCallback);

	mMainScreenFBO.generate();
	mMainScreenFBO.attachColourBuffer(mWindow.mWidth, mWindow.mHeight);
	mMainScreenFBO.attachDepthBuffer(mWindow.mWidth, mWindow.mHeight);

	// Set the initial viewport size for the FBO
	mMainScreenFBO.bind();
	glViewport(0, 0, mWindow.mWidth, mWindow.mHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	OpenGLInstances.push_back(this);
	LOG_INFO("Constructed new OpenGLAPI instance");
}

OpenGLAPI::~OpenGLAPI()
{
	if (mGLADContext && OpenGLInstances.size() == 1)
	{
		free(mGLADContext);
		LOG_INFO("Final OpenGLAPI destructor called. Freeing GLAD memory.");
	}

	auto it = std::find(OpenGLInstances.begin(), OpenGLInstances.end(), this);
	if (it != OpenGLInstances.end())
		OpenGLInstances.erase(it);
}

void OpenGLAPI::preDraw()
{
	mMainScreenFBO.bind();
	mMainScreenFBO.clearBuffers();

	mProjection = glm::perspective(glm::radians(mFOV), mWindow.mAspectRatio, mZNearPlane, mZFarPlane);

	if (mBufferDrawType == GLType::BufferDrawType::Depth)
	{
		mShaders[mDepthViewerIndex].use();
		mShaders[mDepthViewerIndex].setUniform("near", mZNearPlane);
		mShaders[mDepthViewerIndex].setUniform("far",  mZFarPlane);
		mShaders[mDepthViewerIndex].setUniform("linearDepthView",  mLinearDepthView);
	}

	{ // PostProcessing setters
		mShaders[mScreenTextureIndex].use();
		mShaders[mScreenTextureIndex].setUniform("invertColours", mPostProcessingOptions.mInvertColours);
		mShaders[mScreenTextureIndex].setUniform("grayScale", mPostProcessingOptions.mGrayScale);
		mShaders[mScreenTextureIndex].setUniform("sharpen", mPostProcessingOptions.mSharpen);
		mShaders[mScreenTextureIndex].setUniform("blur", mPostProcessingOptions.mBlur);
		mShaders[mScreenTextureIndex].setUniform("edgeDetection", mPostProcessingOptions.mEdgeDetection);
		mShaders[mScreenTextureIndex].setUniform("offset", mPostProcessingOptions.mKernelOffset);
	}

	// TODO: Set this for all shaders that use viewPosition.
	mShaders[mLightMapIndex].use();
	mShaders[mLightMapIndex].setUniform("viewPosition", mViewPosition);
}

void OpenGLAPI::draw(const DrawCall& pDrawCall)
{
	const OpenGLMesh& GLMesh = getGLMesh(pDrawCall.mMesh); // Grab the OpenGLMesh for the Zephyr Mesh requested in the DrawCall.
	const Shader *shader = nullptr;

	if (mBufferDrawType == GLType::BufferDrawType::Colour)
	{
		// #OPTIMIZATION: Future per component combination for entity
		switch (pDrawCall.mDrawStyle)
		{
		case DrawStyle::Textured:
			if (pDrawCall.mTexture1.has_value() && pDrawCall.mTexture2.has_value())
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

			glActiveTexture(GL_TEXTURE0);
			if (pDrawCall.mTexture1.has_value())
				getTexture(pDrawCall.mTexture1.value()).bind();
			else
				getTexture(mMissingTextureID).bind();

			glActiveTexture(GL_TEXTURE1); // || GL_TEXTURE0 + 1
			if (pDrawCall.mTexture2.has_value())
				getTexture(pDrawCall.mTexture2.value()).bind();
			else
				getTexture(mMissingTextureID).bind();

			break;
		case DrawStyle::UniformColour:
			shader = &mShaders[mUniformShaderIndex];
			shader->use();
			shader->setUniform("colour", pDrawCall.mColour.value());
			break;
		case DrawStyle::LightMap:
			ZEPHYR_ASSERT(GLMesh.mDrawSize == 0 || GLMesh.mVBOs[util::toIndex(Shader::Attribute::Normal3D)].has_value(), "Cannot draw a mesh with no Normal data using lighting.")

			shader = &mShaders[mLightMapIndex];
			shader->use();

			glActiveTexture(GL_TEXTURE0);
			if (pDrawCall.mDiffuseTextureID.has_value())
				getTexture(pDrawCall.mDiffuseTextureID.value()).bind();
			else
				getTexture(mMissingTextureID).bind();

			glActiveTexture(GL_TEXTURE1); // || GL_TEXTURE0 + 1
			if (pDrawCall.mSpecularTextureID.has_value())
				getTexture(pDrawCall.mSpecularTextureID.value()).bind();
			else
				getTexture(mMissingTextureID).bind();

			shader->setUniform("lightMap.shininess", pDrawCall.mShininess.value());

			if (pDrawCall.mTextureRepeatFactor.has_value() && (pDrawCall.mDiffuseTextureID.has_value() || pDrawCall.mSpecularTextureID.has_value()))
				shader->setUniform("textureRepeatFactor", pDrawCall.mTextureRepeatFactor.value());
			else
				shader->setUniform("textureRepeatFactor", 1.f);

			break;
		default:
			break;
		}
	}
	else if (mBufferDrawType == GLType::BufferDrawType::Depth)
	{
		shader = &mShaders[mDepthViewerIndex];
		shader->use();
	}
	ZEPHYR_ASSERT(shader != nullptr, "Shader to draw with has not been set.")

	glm::mat4 trans = glm::translate(glm::mat4(1.0f), pDrawCall.mPosition);
	trans = glm::rotate(trans, glm::radians(pDrawCall.mRotation.x), glm::vec3(1.0, 0.0, 0.0));
	trans = glm::rotate(trans, glm::radians(pDrawCall.mRotation.y), glm::vec3(0.0, 1.0, 0.0));
	trans = glm::rotate(trans, glm::radians(pDrawCall.mRotation.z), glm::vec3(0.0, 0.0, 1.0));
	trans = glm::scale(trans, pDrawCall.mScale);

	{
		shader->setUniform("model", trans);
		 // #OPTIMIZATION: view and projection only set when they change (camera + )
		shader->setUniform("view", mViewMatrix);
		shader->setUniform("projection", mProjection);
	}
	glPolygonMode(GL_FRONT_AND_BACK, getPolygonMode(pDrawCall.mDrawMode));

	draw(GLMesh);
}
void OpenGLAPI::draw(const OpenGLAPI::OpenGLMesh& pMesh)
{
	if (pMesh.mDrawSize > 0)
	{
		pMesh.mVAO.bind();

		if (pMesh.mDrawMethod == OpenGLMesh::DrawMethod::Indices)
			glDrawElements(pMesh.mDrawMode, static_cast<GLsizei>(pMesh.mDrawSize), GL_UNSIGNED_INT, 0);
		else if (pMesh.mDrawMethod == OpenGLMesh::DrawMethod::Array)
			glDrawArrays(pMesh.mDrawMode, 0, static_cast<GLsizei>(pMesh.mDrawSize));
	}

	for (const auto& childMesh : pMesh.mChildMeshes)
		draw(childMesh);
}
void OpenGLAPI::draw(const PointLight& pPointLight)
{
	const std::string uniform = "pointLights[" + std::to_string(pointLightDrawCount) + "]";
	const glm::vec3 diffuseColour = pPointLight.mColour * pPointLight.mDiffuseIntensity;
	const glm::vec3 ambientColour = diffuseColour * pPointLight.mAmbientIntensity;
	mShaders[mLightMapIndex].use();
	mShaders[mLightMapIndex].setUniform((uniform + ".position").c_str(), pPointLight.mPosition);
	mShaders[mLightMapIndex].setUniform((uniform + ".ambient").c_str(), ambientColour);
	mShaders[mLightMapIndex].setUniform((uniform + ".diffuse").c_str(), diffuseColour);
	mShaders[mLightMapIndex].setUniform((uniform + ".specular").c_str(), glm::vec3(pPointLight.mSpecularIntensity));
	mShaders[mLightMapIndex].setUniform((uniform + ".constant").c_str(), pPointLight.mConstant);
	mShaders[mLightMapIndex].setUniform((uniform + ".linear").c_str(), pPointLight.mLinear);
	mShaders[mLightMapIndex].setUniform((uniform + ".quadratic").c_str(), pPointLight.mQuadratic);

	pointLightDrawCount++;
}
void OpenGLAPI::draw(const DirectionalLight& pDirectionalLight)
{
	const glm::vec3 diffuseColour = pDirectionalLight.mColour * pDirectionalLight.mDiffuseIntensity;
	const glm::vec3 ambientColour = diffuseColour * pDirectionalLight.mAmbientIntensity;
	mShaders[mLightMapIndex].use();
	mShaders[mLightMapIndex].setUniform("dirLight.direction", pDirectionalLight.mDirection);
	mShaders[mLightMapIndex].setUniform("dirLight.ambient", ambientColour);
	mShaders[mLightMapIndex].setUniform("dirLight.diffuse", diffuseColour);
	mShaders[mLightMapIndex].setUniform("dirLight.specular", glm::vec3(pDirectionalLight.mSpecularIntensity));

	directionalLightDrawCount++;
}
void OpenGLAPI::draw(const SpotLight& pSpotLight)
{
	const glm::vec3 diffuseColour = pSpotLight.mColour * pSpotLight.mDiffuseIntensity;
	const glm::vec3 ambientColour = diffuseColour * pSpotLight.mAmbientIntensity;
	mShaders[mLightMapIndex].use();
	mShaders[mLightMapIndex].setUniform("spotLight.position", pSpotLight.mPosition);
	mShaders[mLightMapIndex].setUniform("spotLight.direction", pSpotLight.mDirection);
	mShaders[mLightMapIndex].setUniform("spotLight.diffuse", diffuseColour);
	mShaders[mLightMapIndex].setUniform("spotLight.ambient", ambientColour);
	mShaders[mLightMapIndex].setUniform("spotLight.specular", glm::vec3(pSpotLight.mSpecularIntensity));
	mShaders[mLightMapIndex].setUniform("spotLight.constant", pSpotLight.mConstant);
	mShaders[mLightMapIndex].setUniform("spotLight.linear", pSpotLight.mLinear);
	mShaders[mLightMapIndex].setUniform("spotLight.quadratic", pSpotLight.mQuadratic);
	mShaders[mLightMapIndex].setUniform("spotLight.cutOff", pSpotLight.mCutOff);
	mShaders[mLightMapIndex].setUniform("spotLight.cutOff", pSpotLight.mOuterCutOff);

	spotLightDrawCount++;
}

void OpenGLAPI::postDraw()
{
	// Unbind after completing draw to ensure all subsequent actions apply to the default FBO.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	{ // Draw the colour output to the screen.
		// Disable culling and depth testing to draw a quad in normalised screen coordinates
		// using the mMainScreenFBO colour-buffer filled in the Draw functions in the last frame.
		GLState previousState = mGLState;
		mGLState.toggleCullFaces(false);
		mGLState.toggleDepthTest(false);

		ZEPHYR_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "mMainScreenFBO not complete, have you called attachColourBuffer and/or attachDepthBuffer");

		mShaders[mScreenTextureIndex].use();
		glActiveTexture(GL_TEXTURE0);
		mMainScreenFBO.getColourTexture().bind();
		draw(getGLMesh(mScreenQuad));

		mGLState = previousState;
	}

	ZEPHYR_ASSERT(pointLightDrawCount == 4, "Only an exact number of 4 pointlights is supported.");
	ZEPHYR_ASSERT(directionalLightDrawCount == 1, "Only one directional light is supported.");
	ZEPHYR_ASSERT(spotLightDrawCount == 1, "Only one spotlight light is supported.");
	pointLightDrawCount = 0;
	directionalLightDrawCount = 0;
	spotLightDrawCount = 0;
}

void OpenGLAPI::endFrame()
{
	mWindow.swapBuffers();
}

void OpenGLAPI::newImGuiFrame()
{
	mWindow.startImGuiFrame();
}
void OpenGLAPI::renderImGuiFrame()
{
	mWindow.renderImGui();
}
void OpenGLAPI::renderImGui()
{
	if (ImGui::Begin("OpenGL options", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(("OpenGL version: " + std::to_string(cOpenGLVersionMajor) + "." + std::to_string(cOpenGLVersionMinor)).c_str());
		ImGui::Text(("Viewport size: " + std::to_string(mWindow.mWidth) + "x" + std::to_string(mWindow.mHeight)).c_str());
		ImGui::Text(("Aspect ratio: " + std::to_string(static_cast<float>(mWindow.mWidth) /  static_cast<float>(mWindow.mHeight))).c_str());
		ImGui::Text(("View position: " + std::to_string(mViewPosition.x) + "," + std::to_string(mViewPosition.y) + "," + std::to_string(mViewPosition.z)).c_str());
		ImGui::SliderFloat("Field of view", &mFOV, 1.f, 120.f);
		ImGui::SliderFloat("Z near plane", &mZNearPlane, 0.001f, 15.f);
		ImGui::SliderFloat("Z far plane", &mZFarPlane, 15.f, 300.f);

		if (ImGui::BeginCombo("Buffer draw style", GLType::toString(mBufferDrawType).c_str(), ImGuiComboFlags()))
    	{
			for (size_t i = 0; i < GLType::bufferDrawTypes.size(); i++)
    	    {
    	        if (ImGui::Selectable(GLType::bufferDrawTypes[i].c_str()))
    	            mBufferDrawType = static_cast<GLType::BufferDrawType>(i);
    	    }
    	    ImGui::EndCombo();
    	}

    	if (mBufferDrawType == GLType::BufferDrawType::Depth)
    	    ImGui::Checkbox("Show linear depth testing", &mLinearDepthView);

		ImGui::Separator();
		mGLState.renderImGui();

    	ImGui::Separator();
		if (ImGui::TreeNode("PostProcessing"))
		{
			ImGui::Checkbox("Invert", 	 	  &mPostProcessingOptions.mInvertColours);
			ImGui::Checkbox("Grayscale", 	  &mPostProcessingOptions.mGrayScale);
			ImGui::Checkbox("Sharpen", 	 	  &mPostProcessingOptions.mSharpen);
			ImGui::Checkbox("Blur", 	 	  &mPostProcessingOptions.mBlur);
			ImGui::Checkbox("Edge detection", &mPostProcessingOptions.mEdgeDetection);

			if (mPostProcessingOptions.mSharpen || mPostProcessingOptions.mBlur || mPostProcessingOptions.mEdgeDetection)
				ImGui::SliderFloat("Kernel offset", &mPostProcessingOptions.mKernelOffset, -1.f, 1.f);

			ImGui::TreePop();
		}
	}
	ImGui::End();
}

const OpenGLAPI::OpenGLMesh& OpenGLAPI::getGLMesh(const MeshID& pMeshID) const
{
	const auto it = mGLMeshes.find(pMeshID);
	ZEPHYR_ASSERT(it != mGLMeshes.end(), "No draw info found for this Mesh ID. Was the mesh correctly initialised?");
	return it->second;
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
std::optional<GLData::VBO> OpenGLAPI::bufferAttributeData(const std::vector<T>& pData, const Shader::Attribute& pAttribute)
{
	if (!pData.empty())
	{
		GLData::VBO vbo;
		vbo.generate();
		vbo.bind();
		glBufferData(GL_ARRAY_BUFFER, pData.size() * sizeof(T), &pData.front(), GL_STATIC_DRAW);
		const GLint attributeIndex = static_cast<GLint>(Shader::getAttributeLocation(pAttribute));
		const GLint attributeComponentCount = static_cast<GLint>(Shader::getAttributeComponentCount(pAttribute));
		glVertexAttribPointer(attributeIndex, attributeComponentCount, getGLFWType<T>(), GL_FALSE, attributeComponentCount * sizeof(T), (void *)0);
		glEnableVertexAttribArray(attributeIndex);
		return vbo;
	}
	else
		return std::nullopt;
}

void OpenGLAPI::initialiseMesh(const Mesh& pMesh)
{
	OpenGLMesh* newMesh = nullptr;
	{
		auto GLMeshLocation = mGLMeshes.find(pMesh.getID());

		if (GLMeshLocation != mGLMeshes.end())
		{
			GLMeshLocation->second.mChildMeshes.push_back({});
			newMesh = &GLMeshLocation->second.mChildMeshes.back();
		}
		else
		{
			mGLMeshes.emplace(std::make_pair(pMesh.getID(), OpenGLMesh()));
			newMesh = &mGLMeshes[pMesh.getID()];

			if (pMesh.mName == "Quad")
				mScreenQuad = pMesh.getID();
		}
	}
	ZEPHYR_ASSERT(newMesh != nullptr, "newMesh not initialised successfully");

	newMesh->mDrawMode = GL_TRIANGLES; // OpenGLAPI only supports GL_TRIANGLES at this revision

	if (!pMesh.mIndices.empty())
	{
		newMesh->mDrawMethod = OpenGLMesh::DrawMethod::Indices;
		newMesh->mDrawSize = static_cast<int>(pMesh.mIndices.size());
	}
	else
	{
		newMesh->mDrawMethod = OpenGLMesh::DrawMethod::Array;
		ZEPHYR_ASSERT(newMesh->mDrawMode == GL_TRIANGLES, "Only GL_TRIANGLES is supported")
		newMesh->mDrawSize = static_cast<int>(pMesh.mVertices.size()) / 3; // Divide verts by 3 as we draw the vertices by triangle count.
	}

	newMesh->mVAO.generate();
	newMesh->mVAO.bind(); // Have to bind VAO before buffering VBO and EBO data

	if (!pMesh.mIndices.empty())
	{
		newMesh->mEBO.generate();
		newMesh->mEBO.bind();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, pMesh.mIndices.size() * sizeof(int), &pMesh.mIndices.front(), GL_STATIC_DRAW);
	}

	newMesh->mVBOs[util::toIndex(Shader::Attribute::Position3D)] 			= bufferAttributeData<float>(pMesh.mVertices, Shader::Attribute::Position3D);
	newMesh->mVBOs[util::toIndex(Shader::Attribute::Normal3D)] 				= bufferAttributeData<float>(pMesh.mNormals, Shader::Attribute::Normal3D);
	newMesh->mVBOs[util::toIndex(Shader::Attribute::ColourRGB)] 			= bufferAttributeData<float>(pMesh.mColours, Shader::Attribute::ColourRGB);
	newMesh->mVBOs[util::toIndex(Shader::Attribute::TextureCoordinate2D)] 	= bufferAttributeData<float>(pMesh.mTextureCoordinates, Shader::Attribute::TextureCoordinate2D);

	LOG_INFO("Zephyr mesh: '{}' Mesh ID: {} loaded into OpenGL with VAO {}", pMesh.mName, pMesh.getID(), newMesh->mVAO.getHandle());

	for (const auto& childMesh : pMesh.mChildMeshes)
		initialiseMesh(childMesh);
}

const GLData::Texture& OpenGLAPI::getTexture(const TextureID& pTextureID) const
{
	ZEPHYR_ASSERT(pTextureID < mTextures.size(), "Trying to access a texture off the end of OpenGL texture store.", pTextureID)
	return mTextures[pTextureID];
}

void OpenGLAPI::initialiseTexture(const Texture& pTexture)
{
	GLData::Texture newTexture;
	newTexture.generate();
	newTexture.bind();
	newTexture.pushData(pTexture.mWidth, pTexture.mHeight, pTexture.mNumberOfChannels, pTexture.getData());
	mTextures[pTexture.getID()] = { newTexture };

	// Cache the ID of the 'missing' texture.
	if (pTexture.mName == "missing")
		mMissingTextureID = pTexture.getID();

    ZEPHYR_ASSERT(newTexture.getHandle() != -1, "Texture {} failed to load", pTexture.mName);
	LOG_INFO("Texture '{}' loaded given ID: {}", pTexture.mName, newTexture.getHandle());
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

GladGLContext* OpenGLAPI::initialiseGLAD()
{
	GladGLContext* GLADContext = (GladGLContext *)malloc(sizeof(GladGLContext));
	int version = gladLoadGLContext(GLADContext, glfwGetProcAddress);
	ZEPHYR_ASSERT(GLADContext && version != 0, "Failed to initialise GLAD GL context")
	// TODO: Add an assert here for GLAD_VERSION to equal to cOpenGLVersion
	LOG_INFO("Initialised GLAD using OpenGL {}.{}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
	return GLADContext;
}

void OpenGLAPI::onResize(const int pWidth, const int pHeight)
{
	mMainScreenFBO.resize(pWidth, pHeight);
	mWindow.mWidth = pWidth;
	mWindow.mHeight = pHeight;
	mWindow.mAspectRatio = static_cast<float>(pWidth) /  static_cast<float>(pHeight);
}

void OpenGLAPI::windowSizeCallback(GLFWwindow* pWindow, int pWidth, int pHeight)
{
	const float width = static_cast<float>(pWidth);
	const float height = static_cast<float>(pHeight);
	LOG_INFO("Window resolution changed to {}x{}", pWidth, pHeight);

	ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2(width, height);
	io.FontGlobalScale = std::round(ImGui::GetMainViewport()->DpiScale);

	for (auto *OpenGLinstance : OpenGLInstances)
	{
		if (OpenGLinstance)
		{
			OpenGLinstance->onResize(pWidth, pHeight);
		}
	}
}