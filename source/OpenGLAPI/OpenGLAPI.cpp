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
	, mWindowClearColour{0.0f, 0.0f, 0.0f}
	, mDepthTest(true)
	, mBufferDrawType(BufferDrawType::Colour)
	, mLinearDepthView(false)
	, mDepthTestType(DepthTestType::Less)
	, mBufferClearBitField(GL_COLOR_BUFFER_BIT)
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
	, mDepthViewerIndex(6)
	, mMissingTextureID(0)
	, pointLightDrawCount(0)
	, spotLightDrawCount(0)
	, directionalLightDrawCount(0)
	, mShaders{ Shader("texture1"), Shader("texture2"), Shader("material"), Shader("colour"), Shader("uniformColour"), Shader("lightMap"), Shader("depthView") }
{
    glfwSetWindowSizeCallback(mWindow.mHandle, windowSizeCallback);
	glViewport(0, 0, mWindow.mWidth, mWindow.mHeight);

	setDepthTest(mDepthTest);
	if (mDepthTest)
		setDepthTestType(mDepthTestType);

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

void OpenGLAPI::onFrameStart()
{
	clearBuffers();
	mWindow.startImGuiFrame();

	if (ImGui::Begin("OpenGL options", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(("OpenGL version: " + std::to_string(cOpenGLVersionMajor) + "." + std::to_string(cOpenGLVersionMinor)).c_str());
		ImGui::Text(("Viewport size: " + std::to_string(mWindow.mWidth) + "x" + std::to_string(mWindow.mHeight)).c_str());
		ImGui::Text(("View position: " + std::to_string(mViewPosition.x) + "," + std::to_string(mViewPosition.y) + "," + std::to_string(mViewPosition.z)).c_str());

		if (ImGui::ColorEdit3("Window clear colour", mWindowClearColour))
			setClearColour(mWindowClearColour[0], mWindowClearColour[1], mWindowClearColour[2]);
		if (ImGui::Checkbox("Depth test", &mDepthTest))
			setDepthTest(mDepthTest);
		if(mDepthTest)
		{
			if (ImGui::BeginCombo("Depth test type", convert(mDepthTestType).c_str(), ImGuiComboFlags()))
			{
				for (size_t i = 0; i < depthTestTypes.size(); i++)
				{
					if (ImGui::Selectable(depthTestTypes[i].c_str()))
						setDepthTestType(static_cast<DepthTestType>(i));
				}
				ImGui::EndCombo();
			}
		}

		if (ImGui::BeginCombo("Buffer draw style", convert(mBufferDrawType).c_str(), ImGuiComboFlags()))
		{
			for (size_t i = 0; i < BufferDrawTypes.size(); i++)
			{
				if (ImGui::Selectable(BufferDrawTypes[i].c_str()))
					mBufferDrawType = static_cast<BufferDrawType>(i);
			}
			ImGui::EndCombo();
		}

		if (mBufferDrawType == BufferDrawType::Depth)
		{
			ImGui::Checkbox("Use linear depth testing" , &mLinearDepthView);
		}


		ImGui::SliderFloat("Field of view", &mFOV, 1.f, 120.f);
		ImGui::SliderFloat("Z near plane", &mZNearPlane, 0.001f, 15.f);
		ImGui::SliderFloat("Z far plane", &mZFarPlane, 15.f, 300.f);
	}
	ImGui::End();

	mProjection = glm::perspective(glm::radians(mFOV), mWindow.mAspectRatio, mZNearPlane, mZFarPlane);
	if (mBufferDrawType == BufferDrawType::Depth)
	{
		mShaders[mDepthViewerIndex].use();
		mShaders[mDepthViewerIndex].setUniform("near", mZNearPlane);
		mShaders[mDepthViewerIndex].setUniform("far",  mZFarPlane);
		mShaders[mDepthViewerIndex].setUniform("linearDepthView",  mLinearDepthView);
	}

	// TODO: Set this for all shaders that use viewPosition.
	mShaders[mLightMapIndex].use();
	mShaders[mLightMapIndex].setUniform("viewPosition", mViewPosition);
}

void OpenGLAPI::draw(const DrawCall& pDrawCall)
{
	const Shader *shader = nullptr;

	if (mBufferDrawType == BufferDrawType::Colour)
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

			if (pDrawCall.mTexture1.has_value())
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, getTextureHandle(pDrawCall.mTexture1.value()));
			}
			else
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, getTextureHandle(mMissingTextureID));
			}
			if (pDrawCall.mTexture2.has_value())
			{
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, getTextureHandle(pDrawCall.mTexture2.value()));
			}
			else
			{
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, getTextureHandle(mMissingTextureID));
			}
			break;
		case DrawStyle::UniformColour:
			shader = &mShaders[mUniformShaderIndex];
			shader->use();
			shader->setUniform("colour", pDrawCall.mColour.value());
			break;
		case DrawStyle::LightMap:
			shader = &mShaders[mLightMapIndex];
			shader->use();

			glActiveTexture(GL_TEXTURE0);
			if (pDrawCall.mDiffuseTextureID.has_value())
				glBindTexture(GL_TEXTURE_2D, getTextureHandle(pDrawCall.mDiffuseTextureID.value()));
			else
				glBindTexture(GL_TEXTURE_2D, getTextureHandle(mMissingTextureID));

			glActiveTexture(GL_TEXTURE1);
			if (pDrawCall.mSpecularTextureID.has_value())
				glBindTexture(GL_TEXTURE_2D, getTextureHandle(pDrawCall.mSpecularTextureID.value()));
			else
				glBindTexture(GL_TEXTURE_2D, getTextureHandle(mMissingTextureID));
				shader->setUniform("lightMap.shininess", pDrawCall.mShininess.value());
			break;
		default:
			break;
		}
	}
	else if (mBufferDrawType == BufferDrawType::Depth)
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

	const OpenGLMesh& GLMesh = getGLMesh(pDrawCall.mMesh); // Grab the OpenGLMesh for the Zephyr Mesh requested in the DrawCall.
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
	mWindow.renderImGui();
	mWindow.swapBuffers();

	ZEPHYR_ASSERT(pointLightDrawCount == 4, "Only an exact number of 4 pointlights is supported.");
	ZEPHYR_ASSERT(directionalLightDrawCount == 1, "Only one directional light is supported.");
	ZEPHYR_ASSERT(spotLightDrawCount == 1, "Only one spotlight light is supported.");

	pointLightDrawCount = 0;
	directionalLightDrawCount = 0;
	spotLightDrawCount = 0;
}

const OpenGLAPI::OpenGLMesh& OpenGLAPI::getGLMesh(const MeshID& pMeshID)
{
	const auto it = mGLMeshes.find(pMeshID);
	ZEPHYR_ASSERT(it != mGLMeshes.end(), "No draw info found for this Mesh ID. Was the mesh correctly initialised?");
	return it->second;
}

OpenGLAPI::TextureHandle OpenGLAPI::getTextureHandle(const TextureID& pTextureID) const
{
	ZEPHYR_ASSERT(pTextureID < mTextures.size(), "Trying to access a texture off the end of OpenGL texture store.", pTextureID)
	return mTextures[pTextureID];
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
		VBO vbo;
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
		}
	}
	ZEPHYR_ASSERT(newMesh != nullptr, "newMesh not initialised successfully");

	newMesh->mDrawMode = GL_TRIANGLES; // OpenGLAPI only supports GL_TRIANGLES at this revision
	newMesh->mDrawMethod = pMesh.mIndices.empty() ? OpenGLMesh::DrawMethod::Array : OpenGLMesh::DrawMethod::Indices;
	newMesh->mDrawSize = pMesh.mIndices.empty() ? static_cast<int>(pMesh.mVertices.size()) : static_cast<int>(pMesh.mIndices.size());

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

void OpenGLAPI::initialiseTexture(const Texture& pTexture)
{
	unsigned int textureHandle;
	glGenTextures(1, &textureHandle);
	glBindTexture(GL_TEXTURE_2D, textureHandle);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLenum format = 0;
	if (pTexture.mNumberOfChannels == 1)
        format = GL_RED;
    else if (pTexture.mNumberOfChannels == 3)
        format = GL_RGB;
    else if (pTexture.mNumberOfChannels == 4)
        format = GL_RGBA;
	ZEPHYR_ASSERT(format != 0, "Could not find channel type for this number of texture channels")

	glTexImage2D(GL_TEXTURE_2D, 0, format, pTexture.mWidth, pTexture.mHeight, 0, format, GL_UNSIGNED_BYTE, pTexture.getData());
	glGenerateMipmap(GL_TEXTURE_2D);
	ZEPHYR_ASSERT(textureHandle != -1, "Texture {} failed to load", pTexture.mName);

	mTextures[pTexture.getID()] = { textureHandle };

	if (pTexture.mName == "missing")
		mMissingTextureID = pTexture.getID();

	LOG_INFO("Texture '{}' loaded given ID: {}", pTexture.mName, textureHandle);
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

void OpenGLAPI::setDepthTest(const bool& pDepthTest)
{
	if (pDepthTest)
	{
		glEnable(GL_DEPTH_TEST);
		mDepthTest = true;
		mBufferClearBitField |= GL_DEPTH_BUFFER_BIT;
	}
	else
	{
		mDepthTest = false;
		glDisable(GL_DEPTH_TEST);
		mGLADContext->Clear(GL_DEPTH_BUFFER_BIT);
		mBufferClearBitField &= ~GL_DEPTH_BUFFER_BIT;
	}
}


void OpenGLAPI::setDepthTestType(const OpenGLAPI::DepthTestType& pType)
{
	ZEPHYR_ASSERT(mDepthTest, "Depth test has to be on to allow setting the depth testing type.");

	mDepthTestType = pType;
	switch (mDepthTestType)
	{
	case OpenGLAPI::DepthTestType::Always:		 glDepthFunc(GL_ALWAYS); 	return;
	case OpenGLAPI::DepthTestType::Never:		 glDepthFunc(GL_NEVER); 	return;
	case OpenGLAPI::DepthTestType::Less:		 glDepthFunc(GL_LESS); 		return;
	case OpenGLAPI::DepthTestType::Equal:		 glDepthFunc(GL_EQUAL);	 	return;
	case OpenGLAPI::DepthTestType::LessEqual:	 glDepthFunc(GL_LEQUAL	);  return;
	case OpenGLAPI::DepthTestType::Greater:		 glDepthFunc(GL_GREATER); 	return;
	case OpenGLAPI::DepthTestType::NotEqual:	 glDepthFunc(GL_NOTEQUAL); 	return;
	case OpenGLAPI::DepthTestType::GreaterEqual: glDepthFunc(GL_GEQUAL); 	return;
	default: ZEPHYR_ASSERT(false, "Unknown DepthTestType requested");		return;
	}
}

void OpenGLAPI::clearBuffers()
{
	mGLADContext->Clear(mBufferClearBitField);
}

void OpenGLAPI::setClearColour(const float &pRed, const float &pGreen, const float &pBlue)
{
	mGLADContext->ClearColor(pRed, pGreen, pBlue, 1.0f);
}

void OpenGLAPI::VAO::generate()
{
	ZEPHYR_ASSERT(!mInitialised, "Calling generate on an already generated VAO")
	glGenVertexArrays(1, &mHandle);
	mInitialised = true;
}
void OpenGLAPI::VAO::bind() const
{
	ZEPHYR_ASSERT(mInitialised, "VAO has not been generated before bind, call glGenVertexArrays before bind");
	glBindVertexArray(mHandle);
}
void OpenGLAPI::VAO::release()
{
	ZEPHYR_ASSERT(mInitialised, "Calling release on an uninitialised VAO");
	glDeleteVertexArrays(1, &mHandle);
}
void OpenGLAPI::VBO::generate()
{
	ZEPHYR_ASSERT(!mInitialised, "Calling generate on an already generated VBO")
	glGenBuffers(1, &mHandle);
	mInitialised = true;
}
void OpenGLAPI::VBO::bind() const
{
	ZEPHYR_ASSERT(mInitialised, "VBO has not been generated before bind, call glGenBuffers before bind");
	glBindBuffer(GL_ARRAY_BUFFER, mHandle);
}
void OpenGLAPI::VBO::release()
{
	ZEPHYR_ASSERT(mInitialised, "Calling release on an uninitialised VBO");
	glDeleteBuffers(1, &mHandle);
}
void OpenGLAPI::EBO::generate()
{
	ZEPHYR_ASSERT(!mInitialised, "Calling generate on an already generated EBO")
	glGenBuffers(1, &mHandle);
	mInitialised = true;
}
void OpenGLAPI::EBO::bind() const
{
	ZEPHYR_ASSERT(mInitialised, "EBO has not been generated before bind, call glGenVertexArrays before bind");
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mHandle);
}
void OpenGLAPI::EBO::release()
{
	ZEPHYR_ASSERT(mInitialised, "Calling release on an uninitialised EBO");
	glDeleteBuffers(1, &mHandle);
}