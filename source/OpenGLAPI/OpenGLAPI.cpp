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
	, cOpenGLVersionMajor(4)
	, cOpenGLVersionMinor(3)
	, mLinearDepthView(false)
	, mVisualiseNormals(false)
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
	, mSkyBoxShaderIndex(8)
	, mVisualiseNormalIndex(9)
	, mScreenQuad(0)
	, mSkyBoxMeshID(0)
	, mMissingTextureID(0)
	, pointLightDrawCount(0)
	, spotLightDrawCount(0)
	, directionalLightDrawCount(0)
	, mBufferDrawType(GLType::BufferDrawType::Colour)
	, mPostProcessingOptions()
	, mShaders{ Shader("texture1", mGLState), Shader("texture2", mGLState), Shader("material", mGLState), Shader("colour", mGLState), Shader("uniformColour", mGLState), Shader("lightMap", mGLState), Shader("depthView", mGLState), Shader("screenTexture", mGLState), Shader("skybox", mGLState), Shader("visualiseNormal", mGLState) }
{
    glfwSetWindowSizeCallback(mWindow.mHandle, windowSizeCallback);

	mMainScreenFBO.generate();
	mMainScreenFBO.attachColourBuffer(mWindow.mWidth, mWindow.mHeight, mGLState);
	mMainScreenFBO.attachDepthBuffer(mWindow.mWidth, mWindow.mHeight, mGLState);

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
	mMainScreenFBO.bind(mGLState);
	mMainScreenFBO.clearBuffers();
	mGLState.checkFramebufferBufferComplete();

	// #OPTIMISATION do this only when view or projection changes.
	mProjection = glm::perspective(glm::radians(mFOV), mWindow.mAspectRatio, mZNearPlane, mZFarPlane);
	mGLState.setUniformBlockVariable("ViewProperties.view", mViewMatrix);
	mGLState.setUniformBlockVariable("ViewProperties.projection", mProjection);

	if (mBufferDrawType == GLType::BufferDrawType::Depth)
	{
		mShaders[mDepthViewerIndex].use(mGLState);
		mShaders[mDepthViewerIndex].setUniform(mGLState, "near", mZNearPlane);
		mShaders[mDepthViewerIndex].setUniform(mGLState, "far",  mZFarPlane);
		mShaders[mDepthViewerIndex].setUniform(mGLState, "linearDepthView",  mLinearDepthView);
	}

	{ // PostProcessing setters
		mShaders[mScreenTextureIndex].use(mGLState);
		mShaders[mScreenTextureIndex].setUniform(mGLState, "invertColours", mPostProcessingOptions.mInvertColours);
		mShaders[mScreenTextureIndex].setUniform(mGLState, "grayScale", mPostProcessingOptions.mGrayScale);
		mShaders[mScreenTextureIndex].setUniform(mGLState, "sharpen", mPostProcessingOptions.mSharpen);
		mShaders[mScreenTextureIndex].setUniform(mGLState, "blur", mPostProcessingOptions.mBlur);
		mShaders[mScreenTextureIndex].setUniform(mGLState, "edgeDetection", mPostProcessingOptions.mEdgeDetection);
		mShaders[mScreenTextureIndex].setUniform(mGLState, "offset", mPostProcessingOptions.mKernelOffset);
	}

	// TODO: Set this for all shaders that use viewPosition.
	mShaders[mLightMapIndex].use(mGLState);
	mShaders[mLightMapIndex].setUniform(mGLState, "viewPosition", mViewPosition);
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
				shader->use(mGLState);
				shader->setUniform(mGLState, "mixFactor", pDrawCall.mMixFactor.value());
			}
			else
			{
				shader = &mShaders[mTexture1ShaderIndex];
				shader->use(mGLState);
			}
			ZEPHYR_ASSERT(shader->getTexturesUnitsCount() > 0, "Shader selected for textured draw does not have any texture units.");

			mGLState.setActiveTextureUnit(0);
			if (pDrawCall.mTexture1.has_value())
				getTexture(pDrawCall.mTexture1.value()).bind();
			else
				getTexture(mMissingTextureID).bind();

			mGLState.setActiveTextureUnit(1);
			if (pDrawCall.mTexture2.has_value())
				getTexture(pDrawCall.mTexture2.value()).bind();
			else
				getTexture(mMissingTextureID).bind();

			break;
		case DrawStyle::UniformColour:
			shader = &mShaders[mUniformShaderIndex];
			shader->use(mGLState);
			shader->setUniform(mGLState, "colour", pDrawCall.mColour.value());
			break;
		case DrawStyle::LightMap:
			ZEPHYR_ASSERT(GLMesh.mDrawSize == 0 || GLMesh.mVBOs[util::toIndex(Shader::Attribute::Normal3D)].has_value(), "Cannot draw a mesh with no Normal data using lighting.")

			shader = &mShaders[mLightMapIndex];
			shader->use(mGLState);

			mGLState.setActiveTextureUnit(0);
			if (pDrawCall.mDiffuseTextureID.has_value())
				getTexture(pDrawCall.mDiffuseTextureID.value()).bind();
			else
				getTexture(mMissingTextureID).bind();

			mGLState.setActiveTextureUnit(1);
			if (pDrawCall.mSpecularTextureID.has_value())
				getTexture(pDrawCall.mSpecularTextureID.value()).bind();
			else
				getTexture(mMissingTextureID).bind();

			shader->setUniform(mGLState, "shininess", pDrawCall.mShininess.value());

			if (pDrawCall.mTextureRepeatFactor.has_value() && (pDrawCall.mDiffuseTextureID.has_value() || pDrawCall.mSpecularTextureID.has_value()))
				shader->setUniform(mGLState, "textureRepeatFactor", pDrawCall.mTextureRepeatFactor.value());
			else
				shader->setUniform(mGLState, "textureRepeatFactor", 1.f);

			break;
		default:
			break;
		}
	}
	else if (mBufferDrawType == GLType::BufferDrawType::Depth)
	{
		shader = &mShaders[mDepthViewerIndex];
		shader->use(mGLState);
	}
	ZEPHYR_ASSERT(shader != nullptr, "Shader to draw with has not been set.")

	glm::mat4 trans = glm::translate(glm::mat4(1.0f), pDrawCall.mPosition);
	trans = glm::rotate(trans, glm::radians(pDrawCall.mRotation.x), glm::vec3(1.0, 0.0, 0.0));
	trans = glm::rotate(trans, glm::radians(pDrawCall.mRotation.y), glm::vec3(0.0, 1.0, 0.0));
	trans = glm::rotate(trans, glm::radians(pDrawCall.mRotation.z), glm::vec3(0.0, 0.0, 1.0));
	trans = glm::scale(trans, pDrawCall.mScale);
	shader->setUniform(mGLState, "model", trans);

	switch (pDrawCall.mDrawMode)
	{
		case DrawMode::Fill: 	  mGLState.setPolygonMode(GLType::PolygonMode::Fill); break;
		case DrawMode::Wireframe: mGLState.setPolygonMode(GLType::PolygonMode::Line); break;
		default:
   			ZEPHYR_ASSERT(false, "Unknown drawMode requested for OpenGLAPI draw.");
			break;
	}

	draw(GLMesh);

	if (mVisualiseNormals)
	{
		mShaders[mVisualiseNormalIndex].setUniform(mGLState, "model", trans);
		draw(GLMesh);
	}
}
void OpenGLAPI::draw(const std::vector<DrawCall>& pDrawCalls)
{
	for (const auto& drawCall : pDrawCalls)
	{
		draw(drawCall);
	}
}
void OpenGLAPI::draw(const OpenGLAPI::OpenGLMesh& pMesh)
{
	if (pMesh.mDrawSize > 0)
	{
		pMesh.mVAO.bind();

		if (pMesh.mDrawMethod == OpenGLMesh::DrawMethod::Indices)
			mGLState.drawElements(pMesh.mDrawMode, pMesh.mDrawSize);
		else if (pMesh.mDrawMethod == OpenGLMesh::DrawMethod::Array)
			mGLState.drawArrays(pMesh.mDrawMode, pMesh.mDrawSize);
	}

	for (const auto& childMesh : pMesh.mChildMeshes)
		draw(childMesh);
}
void OpenGLAPI::draw(const PointLight& pPointLight)
{
	const std::string uniform = "Lights.mPointLights[" + std::to_string(pointLightDrawCount) + "]";
	const glm::vec3 diffuseColour = pPointLight.mColour * pPointLight.mDiffuseIntensity;
	const glm::vec3 ambientColour = diffuseColour * pPointLight.mAmbientIntensity;

	mGLState.setUniformBlockVariable((uniform + ".position").c_str(), pPointLight.mPosition);
	mGLState.setUniformBlockVariable((uniform + ".ambient").c_str(), ambientColour);
	mGLState.setUniformBlockVariable((uniform + ".diffuse").c_str(), diffuseColour);
	mGLState.setUniformBlockVariable((uniform + ".specular").c_str(), glm::vec3(pPointLight.mSpecularIntensity));
	mGLState.setUniformBlockVariable((uniform + ".constant").c_str(), pPointLight.mConstant);
	mGLState.setUniformBlockVariable((uniform + ".linear").c_str(), pPointLight.mLinear);
	mGLState.setUniformBlockVariable((uniform + ".quadratic").c_str(), pPointLight.mQuadratic);

	pointLightDrawCount++;
}
void OpenGLAPI::draw(const DirectionalLight& pDirectionalLight)
{
	const glm::vec3 diffuseColour = pDirectionalLight.mColour * pDirectionalLight.mDiffuseIntensity;
	const glm::vec3 ambientColour = diffuseColour * pDirectionalLight.mAmbientIntensity;

	mGLState.setUniformBlockVariable("Lights.mDirectionalLight.direction", pDirectionalLight.mDirection);
	mGLState.setUniformBlockVariable("Lights.mDirectionalLight.ambient", ambientColour);
	mGLState.setUniformBlockVariable("Lights.mDirectionalLight.diffuse", diffuseColour);
	mGLState.setUniformBlockVariable("Lights.mDirectionalLight.specular", glm::vec3(pDirectionalLight.mSpecularIntensity));

	directionalLightDrawCount++;
}
void OpenGLAPI::draw(const SpotLight& pSpotLight)
{
	const glm::vec3 diffuseColour = pSpotLight.mColour * pSpotLight.mDiffuseIntensity;
	const glm::vec3 ambientColour = diffuseColour * pSpotLight.mAmbientIntensity;

	mGLState.setUniformBlockVariable("Lights.mSpotLight.position", pSpotLight.mPosition);
	mGLState.setUniformBlockVariable("Lights.mSpotLight.direction", pSpotLight.mDirection);
	mGLState.setUniformBlockVariable("Lights.mSpotLight.diffuse", diffuseColour);
	mGLState.setUniformBlockVariable("Lights.mSpotLight.ambient", ambientColour);
	mGLState.setUniformBlockVariable("Lights.mSpotLight.specular", glm::vec3(pSpotLight.mSpecularIntensity));
	mGLState.setUniformBlockVariable("Lights.mSpotLight.constant", pSpotLight.mConstant);
	mGLState.setUniformBlockVariable("Lights.mSpotLight.linear", pSpotLight.mLinear);
	mGLState.setUniformBlockVariable("Lights.mSpotLight.quadratic", pSpotLight.mQuadratic);
	mGLState.setUniformBlockVariable("Lights.mSpotLight.cutOff", pSpotLight.mCutOff);
	mGLState.setUniformBlockVariable("Lights.mSpotLight.cutOff", pSpotLight.mOuterCutOff);

	spotLightDrawCount++;
}

void OpenGLAPI::postDraw()
{
	{ // Skybox render
		// Skybox is drawn in postDraw to maximise depth test culling of the textures in the cubemap which will always pass otherwise.
		// Depth testing must be set to GL_LEQUAL because the depth values of skybox's are equal to depth buffer contents.
		mShaders[mSkyBoxShaderIndex].use(mGLState);
		const glm::mat4 view = glm::mat4(glm::mat3(mViewMatrix)); // remove translation from the view matrix
		mShaders[mSkyBoxShaderIndex].setUniform(mGLState, "viewNoTranslation", view);
		mShaders[mSkyBoxShaderIndex].setUniform(mGLState, "projection", mProjection);

		GLState previousState = mGLState;
		mGLState.toggleDepthTest(true);
		mGLState.setDepthTestType(GLType::DepthTestType::LessEqual);

		mGLState.setActiveTextureUnit(0);
		mCubeMaps.front().bind();
		draw(getGLMesh(mSkyBoxMeshID));

		mGLState = previousState;
	}

	// Unbind after completing draw to ensure all subsequent actions apply to the default FBO.
	mGLState.unbindFramebuffer();

	{ // Draw the colour output to the screen.
		// Disable culling and depth testing to draw a quad in normalised screen coordinates
		// using the mMainScreenFBO colour-buffer filled in the Draw functions in the last frame.
		GLState previousState = mGLState;
		mGLState.toggleCullFaces(false);
		mGLState.toggleDepthTest(false);

		mShaders[mScreenTextureIndex].use(mGLState);
		mGLState.setActiveTextureUnit(0);
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

		ImGui::Checkbox("Visualise normals", &mVisualiseNormals);

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
			else if (pMesh.mName == "Skybox")
				mSkyBoxMeshID = pMesh.getID();
		}
	}
	ZEPHYR_ASSERT(newMesh != nullptr, "newMesh not initialised successfully");

	newMesh->mDrawMode = GLType::PrimitiveMode::Triangles; // OpenGLAPI only supports Triangles at this revision

	if (!pMesh.mIndices.empty())
	{
		newMesh->mDrawMethod = OpenGLMesh::DrawMethod::Indices;
		newMesh->mDrawSize = static_cast<int>(pMesh.mIndices.size());
	}
	else
	{
		newMesh->mDrawMethod = OpenGLMesh::DrawMethod::Array;
		ZEPHYR_ASSERT(newMesh->mDrawMode == GLType::PrimitiveMode::Triangles, "Only PrimitiveMode::Triangles is supported")
		newMesh->mDrawSize = static_cast<int>(pMesh.mVertices.size()) / 3; // Divide verts by 3 as we draw the vertices by Triangles count.
	}

	newMesh->mVAO.generate();
	newMesh->mVAO.bind(); // Have to bind VAO before buffering VBO and EBO data

	if (!pMesh.mIndices.empty())
	{
		newMesh->mEBO.generate();
		newMesh->mEBO.bind();
		newMesh->mEBO.pushData(pMesh.mIndices);
	}

	if (!pMesh.mVertices.empty())
	{
		newMesh->mVBOs[util::toIndex(Shader::Attribute::Position3D)] = GLData::VBO();
		newMesh->mVBOs[util::toIndex(Shader::Attribute::Position3D)]->generate();
		newMesh->mVBOs[util::toIndex(Shader::Attribute::Position3D)]->bind();
		newMesh->mVBOs[util::toIndex(Shader::Attribute::Position3D)]->pushData(pMesh.mVertices, Shader::getAttributeLocation(Shader::Attribute::Position3D), Shader::getAttributeComponentCount(Shader::Attribute::Position3D));
	}
	if (!pMesh.mNormals.empty())
	{
		newMesh->mVBOs[util::toIndex(Shader::Attribute::Normal3D)] = GLData::VBO();
		newMesh->mVBOs[util::toIndex(Shader::Attribute::Normal3D)]->generate();
		newMesh->mVBOs[util::toIndex(Shader::Attribute::Normal3D)]->bind();
		newMesh->mVBOs[util::toIndex(Shader::Attribute::Normal3D)]->pushData(pMesh.mNormals, Shader::getAttributeLocation(Shader::Attribute::Normal3D), Shader::getAttributeComponentCount(Shader::Attribute::Normal3D));
	}
	if (!pMesh.mColours.empty())
	{
		newMesh->mVBOs[util::toIndex(Shader::Attribute::ColourRGB)] = GLData::VBO();
		newMesh->mVBOs[util::toIndex(Shader::Attribute::ColourRGB)]->generate();
		newMesh->mVBOs[util::toIndex(Shader::Attribute::ColourRGB)]->bind();
		newMesh->mVBOs[util::toIndex(Shader::Attribute::ColourRGB)]->pushData(pMesh.mColours, Shader::getAttributeLocation(Shader::Attribute::ColourRGB), Shader::getAttributeComponentCount(Shader::Attribute::ColourRGB));
	}
	if (!pMesh.mTextureCoordinates.empty())
	{
		newMesh->mVBOs[util::toIndex(Shader::Attribute::TextureCoordinate2D)] = GLData::VBO();
		newMesh->mVBOs[util::toIndex(Shader::Attribute::TextureCoordinate2D)]->generate();
		newMesh->mVBOs[util::toIndex(Shader::Attribute::TextureCoordinate2D)]->bind();
		newMesh->mVBOs[util::toIndex(Shader::Attribute::TextureCoordinate2D)]->pushData(pMesh.mTextureCoordinates, Shader::getAttributeLocation(Shader::Attribute::TextureCoordinate2D), Shader::getAttributeComponentCount(Shader::Attribute::TextureCoordinate2D));
	}

	LOG_INFO("OpenGL::Mesh: '{}' with MeshID: {} loaded into OpenGL with VAO: {}", pMesh.mName, pMesh.getID(), newMesh->mVAO.getHandle());

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
	GLData::Texture newTexture = { GLData::Texture::Type::Texture2D };
	newTexture.generate();
	newTexture.bind();
	newTexture.pushData(pTexture.mWidth, pTexture.mHeight, pTexture.mNumberOfChannels, pTexture.getData());
	mTextures[pTexture.getID()] = { newTexture };

	// Cache the ID of the 'missing' texture.
	if (pTexture.mName == "missing")
		mMissingTextureID = pTexture.getID();

    ZEPHYR_ASSERT(newTexture.getHandle() != -1, "Texture {} failed to load", pTexture.mName);
	LOG_INFO("OpenGL::Texture '{}' loaded given VAO: {}", pTexture.mName, newTexture.getHandle());
}

void OpenGLAPI::initialiseCubeMap(const CubeMapTexture& pCubeMap)
{
	// OpenGL cubeMap texture objects store all 6 faces under 1 VAO hence only one generate and bind is used before 6 pushData calls
	// Each face can be offset by index (last param of pushData) in the order Right (0), Left (1), Top(2), Bottom(3), Back(4), Front(5)

	GLData::Texture newCubeMap = { GLData::Texture::Type::CubeMap };
	newCubeMap.generate();
	newCubeMap.bind();

	newCubeMap.pushData(pCubeMap.mRight.mWidth, pCubeMap.mRight.mHeight, pCubeMap.mRight.mNumberOfChannels, pCubeMap.mRight.getData(), 0);
	newCubeMap.pushData(pCubeMap.mLeft.mWidth, pCubeMap.mLeft.mHeight, pCubeMap.mLeft.mNumberOfChannels, pCubeMap.mLeft.getData(), 1);
	newCubeMap.pushData(pCubeMap.mTop.mWidth, pCubeMap.mTop.mHeight, pCubeMap.mTop.mNumberOfChannels, pCubeMap.mTop.getData(), 2);
	newCubeMap.pushData(pCubeMap.mBottom.mWidth, pCubeMap.mBottom.mHeight, pCubeMap.mBottom.mNumberOfChannels, pCubeMap.mBottom.getData(), 3);
	newCubeMap.pushData(pCubeMap.mFront.mWidth, pCubeMap.mFront.mHeight, pCubeMap.mFront.mNumberOfChannels, pCubeMap.mFront.getData(), 4);
	newCubeMap.pushData(pCubeMap.mBack.mWidth, pCubeMap.mBack.mHeight, pCubeMap.mBack.mNumberOfChannels, pCubeMap.mBack.getData(), 5);

	mCubeMaps.push_back(newCubeMap);
	LOG_INFO("OpenGL::CubeMapTexture '{}' loaded given VAO: {}", pCubeMap.mName, newCubeMap.getHandle());
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
	mMainScreenFBO.resize(pWidth, pHeight, mGLState);
	mGLState.setViewport(pWidth, pHeight);
	mWindow.mWidth = pWidth;
	mWindow.mHeight = pHeight;
	mWindow.mAspectRatio = static_cast<float>(pWidth) / static_cast<float>(pHeight);
}

void OpenGLAPI::windowSizeCallback(GLFWwindow* pWindow, int pWidth, int pHeight)
{
	const float width = static_cast<float>(pWidth);
	const float height = static_cast<float>(pHeight);
	LOG_INFO("OpenGL Window resolution changed to {}x{}", pWidth, pHeight);

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