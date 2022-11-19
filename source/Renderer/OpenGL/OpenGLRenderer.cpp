#include "OpenGLRenderer.hpp"

// ECS
#include "Storage.hpp"

// COMPONENTS
#include "DirectionalLight.hpp"
#include "PointLight.hpp"
#include "SpotLight.hpp"
#include "Texture.hpp"
#include "Collider.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"
#include "Transform.hpp"

// SYSTEMS
#include "MeshSystem.hpp"
#include "TextureSystem.hpp"

#include "Logger.hpp"

// EXTERNAL
#include "glad/gl.h"
#include "GLFW/glfw3.h" // Used to initialise GLAD using glfwGetProcAddress
#include "glm/ext/matrix_transform.hpp" // perspective, translate, rotate
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"

// STD
#include <algorithm>

namespace OpenGL
{
    OpenGLRenderer::OpenGLRenderer(ECS::Storage& pStorage, const System::MeshSystem& pMeshSystem, const System::TextureSystem& pTextureSystem)
        : cOpenGLVersionMajor(4)
        , cOpenGLVersionMinor(3)
        , mWindow(cOpenGLVersionMajor, cOpenGLVersionMinor)
        , mGLADContext(initialiseGLAD()) // TODO: This should only happen on first OpenGLRenderer construction (OpenGLInstances.size() == 1)
        , mGLState()
        , mMainScreenFBO()
        , mStorage(pStorage)
        , mViewMatrix()
        , mViewPosition()
        , mProjection()
        , mLinearDepthView(false)
        , mVisualiseNormals(false)
        , mZNearPlane(0.1f)
        , mZFarPlane(100.0f)
        , mFOV(45.f)
        , pointLightDrawCount(0)
        , spotLightDrawCount(0)
        , directionalLightDrawCount(0)
        , mBufferDrawType(BufferDrawType::Colour)
        , mPostProcessingOptions()
        , mAvailableShaders
        { Shader("texture1" , mGLState)
        , Shader("texture2" , mGLState)
        , Shader("material" , mGLState)
        , Shader("colour" , mGLState)
        , Shader("uniformColour" , mGLState)
        , Shader("lightMap" , mGLState)
        , Shader("texture1Instanced" , mGLState)}
        , mTexture1ShaderIndex(0)
        , mTexture2ShaderIndex(1)
        , mMaterialShaderIndex(2)
        , mUniformShaderIndex(4)
        , mLightMapIndex(5)
        , mTexture1InstancedShaderIndex(6)
        , mScreenTextureShader("screenTexture", mGLState)
        , mSkyBoxShader("skybox", mGLState)
        , mLightEmitterShader("uniformColour", mGLState)
        , mDepthViewerShader("depthView", mGLState)
        , mVisualiseNormalShader("visualiseNormal", mGLState)
        , mGLMeshData{}
        , m3DCubeMeshIndex{0}
        , mSkyBoxMeshIndex{0}
        , mScreenQuadMeshIndex{0}
        , mTextures{}
        , mMissingTextureID{0}
        , mCubeMaps{}
    {
        pMeshSystem.ForEach([this](const auto& mesh) { initialiseMesh(mesh); }); // Depends on mShaders being initialised.
        pTextureSystem.ForEach([this](const auto& texture) { initialiseTexture(texture); });
        pTextureSystem.ForEachCubeMap([this](const auto& cubeMap) { initialiseCubeMap(cubeMap); });

        mStorage.foreach([this](Component::Camera& pCamera)
        {
            if (pCamera.mPrimaryCamera)
            {
                mViewMatrix = pCamera.getViewMatrix();
                mViewPosition = pCamera.getPosition();
            }
        });

        glfwSetWindowSizeCallback(mWindow.mHandle, windowSizeCallback);

        mMainScreenFBO.generate();
        mMainScreenFBO.attachColourBuffer(mWindow.mWidth, mWindow.mHeight, mGLState);
        mMainScreenFBO.attachDepthBuffer(mWindow.mWidth, mWindow.mHeight, mGLState);

        OpenGLInstances.push_back(this);
        LOG_INFO("Constructed new OpenGLRenderer instance");
    }

    OpenGLRenderer::~OpenGLRenderer()
    {
        if (mGLADContext && OpenGLInstances.size() == 1)
        {
            free(mGLADContext);
            LOG_INFO("Final OpenGLRenderer destructor called. Freeing GLAD memory.");
        }

        auto it = std::find(OpenGLInstances.begin(), OpenGLInstances.end(), this);
        if (it != OpenGLInstances.end())
            OpenGLInstances.erase(it);
    }

    void OpenGLRenderer::preDraw()
    {
        mStorage.foreach([this](Component::Camera& pCamera)
        {
            if (pCamera.mPrimaryCamera)
            {
                mViewMatrix = pCamera.getViewMatrix();
                mViewPosition = pCamera.getPosition();
            }
        });

        mMainScreenFBO.bind(mGLState);
        mMainScreenFBO.clearBuffers();
        mGLState.checkFramebufferBufferComplete();

        mProjection = glm::perspective(glm::radians(mFOV), mWindow.mAspectRatio, mZNearPlane, mZFarPlane);
        mGLState.setUniformBlockVariable("ViewProperties.view", mViewMatrix);
        mGLState.setUniformBlockVariable("ViewProperties.projection", mProjection);

        if (mBufferDrawType == BufferDrawType::Depth)
        {
            mDepthViewerShader.use(mGLState);
            mDepthViewerShader.setUniform(mGLState, "near", mZNearPlane);
            mDepthViewerShader.setUniform(mGLState, "far", mZFarPlane);
            mDepthViewerShader.setUniform(mGLState, "linearDepthView", mLinearDepthView);
        }

        { // PostProcessing setters
            mScreenTextureShader.use(mGLState);
            mScreenTextureShader.setUniform(mGLState, "invertColours", mPostProcessingOptions.mInvertColours);
            mScreenTextureShader.setUniform(mGLState, "grayScale", mPostProcessingOptions.mGrayScale);
            mScreenTextureShader.setUniform(mGLState, "sharpen", mPostProcessingOptions.mSharpen);
            mScreenTextureShader.setUniform(mGLState, "blur", mPostProcessingOptions.mBlur);
            mScreenTextureShader.setUniform(mGLState, "edgeDetection", mPostProcessingOptions.mEdgeDetection);
            mScreenTextureShader.setUniform(mGLState, "offset", mPostProcessingOptions.mKernelOffset);
        }

       for (size_t i = 0; i < mAvailableShaders.size(); i++)
       {
           if (mAvailableShaders[i].getName() == "lightMap")
               mAvailableShaders[i].setUniform(mGLState, "viewPosition", mViewPosition);
       }
    }

    void OpenGLRenderer::draw()
    {
        mStorage.foreach([this](Component::Transform& pTransform, Component::MeshDraw& pMeshDraw)
        {
            const GLMeshData& GLMesh = mGLMeshData[pMeshDraw.mID.Get()];

            if (mBufferDrawType == BufferDrawType::Colour)
            {
                auto* shader = getShader(pMeshDraw);
                shader->use(mGLState);

                if (shader->getName() == "texture1" || shader->getName() == "texture1Instanced")
                {
                    ZEPHYR_ASSERT(pMeshDraw.mTexture1.has_value(), "DrawCall must have mTexture1 set to draw using texture1 shader");
                    mGLState.setActiveTextureUnit(0);
                    getTexture(pMeshDraw.mTexture1.value()).bind();
                }
                else if (shader->getName() == "texture2")
                {
                    ZEPHYR_ASSERT(pMeshDraw.mMixFactor.has_value(), "DrawCall must have mixFactor set to draw using texture2 shader");
                    ZEPHYR_ASSERT(pMeshDraw.mTexture1.has_value(), "DrawCall must have mTexture1 set to draw using texture2 shader");
                    ZEPHYR_ASSERT(pMeshDraw.mTexture2.has_value(), "DrawCall must have mTexture2 set to draw using texture2 shader");

                    shader->setUniform(mGLState, "mixFactor", pMeshDraw.mMixFactor.value());
                    mGLState.setActiveTextureUnit(0);
                    getTexture(pMeshDraw.mTexture1.value()).bind();
                    mGLState.setActiveTextureUnit(1);
                    getTexture(pMeshDraw.mTexture2.value()).bind();
                }
                else if (shader->getName() == "uniformColour")
                {
                    ZEPHYR_ASSERT(pMeshDraw.mColour.has_value(), "DrawCall must have mColour set to draw using uniformColour shader");
                    shader->setUniform(mGLState, "colour", pMeshDraw.mColour.value());
                }
                else if (shader->getName() == "lightMap")
                {
                    ZEPHYR_ASSERT(GLMesh.mDrawSize == 0 || GLMesh.mVBOs[Utility::toIndex(Shader::Attribute::Normal3D)].has_value(), "Cannot draw a mesh with no Normal data using lightMap shader.");
                    ZEPHYR_ASSERT(pMeshDraw.mDiffuseTextureID.has_value(), "DrawCall must have mDiffuseTextureID set to draw using lightMap shader");
                    ZEPHYR_ASSERT(pMeshDraw.mSpecularTextureID.has_value(), "DrawCall must have mSpecularTextureID set to draw using lightMap shader");
                    ZEPHYR_ASSERT(pMeshDraw.mShininess.has_value(), "DrawCall must have mTexture2 set to draw using texture2");

                    mGLState.setActiveTextureUnit(0);
                    getTexture(pMeshDraw.mDiffuseTextureID.value()).bind();
                    mGLState.setActiveTextureUnit(1);
                    getTexture(pMeshDraw.mSpecularTextureID.value()).bind();
                    shader->setUniform(mGLState, "shininess", pMeshDraw.mShininess.value());
                    shader->setUniform(mGLState, "textureRepeatFactor", pMeshDraw.mTextureRepeatFactor.has_value() ? pMeshDraw.mTextureRepeatFactor.value() : 1.f);
                }
                else if (shader->getName() == "depthView")
                {}

                switch (pMeshDraw.mDrawMode)
                {
                    case Component::DrawMode::Fill:      mGLState.setPolygonMode(GLType::PolygonMode::Fill); break;
                    case Component::DrawMode::Wireframe: mGLState.setPolygonMode(GLType::PolygonMode::Line); break;
                    default: ZEPHYR_ASSERT(false, "Unknown drawMode requested for OpenGLRenderer draw.");    break;
                }

                // Instanced shaders set their models in buffers so dont need to set the model matrix here, just call draw.
                if (shader->isInstanced())
                {
                    ZEPHYR_ASSERT(false, "Instanced rendering is disabled. No instanced shader should be assigned to a MeshDraw.");
                    //draw(GLMesh, pMeshDraw.mModels.size());
                }
                else
                {
                    auto model = Utility::GetModelMatrix(pTransform.mPosition, pTransform.mRotation, pTransform.mScale);
                    shader->setUniform(mGLState, "model", model);
                    draw(GLMesh);
                    if (mVisualiseNormals)
                    {
                        mVisualiseNormalShader.setUniform(mGLState, "model", model);
                        draw(GLMesh);
                    }
                }
            }
            else if (mBufferDrawType == BufferDrawType::Depth)
            {
                //mDepthViewerShader
            }
        });
    }

    void OpenGLRenderer::draw(const OpenGLRenderer::GLMeshData& pMesh, const size_t& pInstancedCount /* = 0*/)
    {
        if (pMesh.mDrawSize > 0)
        {
            pMesh.mVAO.bind();

            if (pInstancedCount > 0)
            {
                if (pMesh.mDrawMethod == GLMeshData::DrawMethod::Indices)
                    mGLState.drawElementsInstanced(pMesh.mDrawMode, pMesh.mDrawSize, static_cast<int>(pInstancedCount));
                else if (pMesh.mDrawMethod == GLMeshData::DrawMethod::Array)
                    mGLState.drawArraysInstanced(pMesh.mDrawMode, pMesh.mDrawSize, static_cast<int>(pInstancedCount));
            }
            else
            {
                if (pMesh.mDrawMethod == GLMeshData::DrawMethod::Indices)
                    mGLState.drawElements(pMesh.mDrawMode, pMesh.mDrawSize);
                else if (pMesh.mDrawMethod == GLMeshData::DrawMethod::Array)
                    mGLState.drawArrays(pMesh.mDrawMode, pMesh.mDrawSize);
            }
        }

        for (const auto& childMesh : pMesh.mChildMeshes)
            draw(childMesh);
    }

    void OpenGLRenderer::setupLights(const bool& pRenderLightPositions)
    {
        mStorage.foreach([this](Component::PointLight& pPointLight)
        {
            setShaderVariables(pPointLight);
        });
        mStorage.foreach([this](Component::DirectionalLight& pDirectionalLight)
        {
            setShaderVariables(pDirectionalLight);
        });
        mStorage.foreach([this](Component::SpotLight& pSpotLight)
        {
            setShaderVariables(pSpotLight);
        });

        if (pRenderLightPositions)
        {
            mLightEmitterShader.use(mGLState);

            mStorage.foreach([this](Component::PointLight& pPointLight)
            {
                mLightEmitterShader.setUniform(mGLState, "model", Utility::GetModelMatrix(pPointLight.mPosition, glm::vec3(0.f), glm::vec3(0.1f)));
                mLightEmitterShader.setUniform(mGLState, "colour", pPointLight.mColour);
                draw(mGLMeshData[m3DCubeMeshIndex]);
            });
        }

        { // Render collision shapes
            mLightEmitterShader.use(mGLState);
            mGLState.setPolygonMode(GLType::PolygonMode::Line);

            mStorage.foreach([this](Component::Collider& pCollider, Component::Transform& pTransform)
            {
			    const auto highPoint = glm::vec3(pCollider.mBoundingBox.mHighX, pCollider.mBoundingBox.mHighY, pCollider.mBoundingBox.mHighZ);
			    const auto lowPoint = glm::vec3(pCollider.mBoundingBox.mLowX, pCollider.mBoundingBox.mLowY, pCollider.mBoundingBox.mLowZ);
			    const auto lowToHigh = highPoint - lowPoint;
			    const glm::vec3 center = lowPoint + (lowToHigh / 2.f);
			    const auto sizeX = pCollider.mBoundingBox.mHighX - pCollider.mBoundingBox.mLowX;
			    const auto sizeY = pCollider.mBoundingBox.mHighY - pCollider.mBoundingBox.mLowY;
			    const auto sizeZ = pCollider.mBoundingBox.mHighZ - pCollider.mBoundingBox.mLowZ;

                auto colliderModelMat = Utility::GetModelMatrix(center, glm::vec3(0.f), glm::vec3(sizeX, sizeY, sizeZ));
                auto transformModelMat = Utility::GetModelMatrix(pTransform.mPosition, pTransform.mRotation, pTransform.mScale);

			    mLightEmitterShader.setUniform(mGLState, "model", colliderModelMat * transformModelMat);
   			    mLightEmitterShader.setUniform(mGLState, "colour", glm::vec3(0.f, 1.f, 0.f));
                draw(mGLMeshData[m3DCubeMeshIndex]);
            });
            mGLState.setPolygonMode(GLType::PolygonMode::Fill);
        }
    }

    Shader* OpenGLRenderer::getShader(Component::MeshDraw& pMeshDraw)
    {
        Shader* shaderToUse = nullptr;
        switch (pMeshDraw.mDrawStyle)
        {
            case Component::DrawStyle::Textured:
                if (pMeshDraw.mTexture1.has_value() && pMeshDraw.mTexture2.has_value())
                    shaderToUse = &mAvailableShaders[mTexture2ShaderIndex];
                else
                    shaderToUse = &mAvailableShaders[mTexture1ShaderIndex];
                break;
            case Component::DrawStyle::UniformColour:
                shaderToUse = &mAvailableShaders[mUniformShaderIndex];
                break;
            case Component::DrawStyle::LightMap:
                shaderToUse = &mAvailableShaders[mLightMapIndex];
                break;
        }
        ZEPHYR_ASSERT(shaderToUse != nullptr, "Couldn't identify a shader to render this MeshDraw with.");
        return shaderToUse;
    }

    void OpenGLRenderer::setShaderVariables(const Component::PointLight& pPointLight)
    {
        const std::string uniform     = "Lights.mPointLights[" + std::to_string(pointLightDrawCount) + "]";
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

    void OpenGLRenderer::setShaderVariables(const Component::DirectionalLight& pDirectionalLight)
    {
        const glm::vec3 diffuseColour = pDirectionalLight.mColour * pDirectionalLight.mDiffuseIntensity;
        const glm::vec3 ambientColour = diffuseColour * pDirectionalLight.mAmbientIntensity;

        mGLState.setUniformBlockVariable("Lights.mDirectionalLight.direction", pDirectionalLight.mDirection);
        mGLState.setUniformBlockVariable("Lights.mDirectionalLight.ambient", ambientColour);
        mGLState.setUniformBlockVariable("Lights.mDirectionalLight.diffuse", diffuseColour);
        mGLState.setUniformBlockVariable("Lights.mDirectionalLight.specular", glm::vec3(pDirectionalLight.mSpecularIntensity));

        directionalLightDrawCount++;
    }
    void OpenGLRenderer::setShaderVariables(const Component::SpotLight& pSpotLight)
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

    void OpenGLRenderer::postDraw()
    {
        { // Skybox render
            // Skybox is drawn in postDraw to maximise depth test culling of the textures in the cubemap which will always pass otherwise.
            // Depth testing must be set to GL_LEQUAL because the depth values of skybox's are equal to depth buffer contents.
            mSkyBoxShader.use(mGLState);
            const glm::mat4 view = glm::mat4(glm::mat3(mViewMatrix)); // remove translation from the view matrix
            mSkyBoxShader.setUniform(mGLState, "viewNoTranslation", view);
            mSkyBoxShader.setUniform(mGLState, "projection", mProjection);

            const bool depthTestBefore                      = mGLState.getDepthTest();
            const GLType::DepthTestType depthTestTypeBefore = mGLState.getDepthTestType();
            mGLState.toggleDepthTest(true);
            mGLState.setDepthTestType(GLType::DepthTestType::LessEqual);

            mGLState.setActiveTextureUnit(0);
            mCubeMaps.front().bind();
            draw(mGLMeshData[mSkyBoxMeshIndex]);

            mGLState.toggleDepthTest(depthTestBefore);
            mGLState.setDepthTestType(depthTestTypeBefore);
        }

        // Unbind after completing draw to ensure all subsequent actions apply to the default FBO.
        mGLState.unbindFramebuffer();

        { // Draw the colour output to the screen.
            // Disable culling and depth testing to draw a quad in normalised screen coordinates
            // using the mMainScreenFBO colour-buffer filled in the Draw functions in the last frame.
            const bool depthTestBefore = mGLState.getDepthTest();
            const bool cullFacesBefore = mGLState.getCullFaces();
            mGLState.toggleCullFaces(false);
            mGLState.toggleDepthTest(false);

            mScreenTextureShader.use(mGLState);
            mGLState.setActiveTextureUnit(0);
            mMainScreenFBO.getColourTexture().bind();
            draw(mGLMeshData[mScreenQuadMeshIndex]);

            mGLState.toggleCullFaces(cullFacesBefore);
            mGLState.toggleDepthTest(depthTestBefore);
        }

        ZEPHYR_ASSERT(pointLightDrawCount == 4, "Only an exact number of 4 pointlights is supported.");
        ZEPHYR_ASSERT(directionalLightDrawCount == 1, "Only one directional light is supported.");
        ZEPHYR_ASSERT(spotLightDrawCount == 1, "Only one spotlight light is supported.");
        pointLightDrawCount       = 0;
        directionalLightDrawCount = 0;
        spotLightDrawCount        = 0;
    }

    void OpenGLRenderer::endFrame()
    {
        mWindow.swapBuffers();
    }
    void OpenGLRenderer::newImGuiFrame()
    {
        mWindow.startImGuiFrame();
    }
    void OpenGLRenderer::renderImGuiFrame()
    {
        mWindow.renderImGui();
    }
    void OpenGLRenderer::renderImGui()
    {
        if (ImGui::Begin("OpenGL options", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text(("OpenGL version: " + std::to_string(cOpenGLVersionMajor) + "." + std::to_string(cOpenGLVersionMinor)).c_str());
            ImGui::Text(("Viewport size: " + std::to_string(mWindow.mWidth) + "x" + std::to_string(mWindow.mHeight)).c_str());
            ImGui::Text(("Aspect ratio: " + std::to_string(static_cast<float>(mWindow.mWidth) / static_cast<float>(mWindow.mHeight))).c_str());
            ImGui::Text(("View position: " + std::to_string(mViewPosition.x) + "," + std::to_string(mViewPosition.y) + "," + std::to_string(mViewPosition.z)).c_str());
            ImGui::SliderFloat("Field of view", &mFOV, 1.f, 120.f);
            ImGui::SliderFloat("Z near plane", &mZNearPlane, 0.001f, 15.f);
            ImGui::SliderFloat("Z far plane", &mZFarPlane, 15.f, 300.f);
            ImGui::Separator();

            static const std::array<std::string, Utility::toIndex(BufferDrawType::Count)> bufferDrawTypes{
                "Colour",
                "Depth"};
            if (ImGui::BeginCombo("Buffer draw style", bufferDrawTypes[static_cast<size_t>(mBufferDrawType)].c_str(), ImGuiComboFlags()))
            {
                for (size_t i = 0; i < bufferDrawTypes.size(); i++)
                {
                    if (ImGui::Selectable(bufferDrawTypes[i].c_str()))
                        mBufferDrawType = static_cast<BufferDrawType>(i);
                }
                ImGui::EndCombo();
            }

            if (mBufferDrawType == BufferDrawType::Depth)
                ImGui::Checkbox("Show linear depth testing", &mLinearDepthView);

            ImGui::Checkbox("Visualise normals", &mVisualiseNormals);

            ImGui::Separator();
            mGLState.renderImGui();
            ImGui::Separator();

            ImGui::Separator();
            if (ImGui::TreeNode("PostProcessing"))
            {
                ImGui::Checkbox("Invert", &mPostProcessingOptions.mInvertColours);
                ImGui::Checkbox("Grayscale", &mPostProcessingOptions.mGrayScale);
                ImGui::Checkbox("Sharpen", &mPostProcessingOptions.mSharpen);
                ImGui::Checkbox("Blur", &mPostProcessingOptions.mBlur);
                ImGui::Checkbox("Edge detection", &mPostProcessingOptions.mEdgeDetection);

                if (mPostProcessingOptions.mSharpen || mPostProcessingOptions.mBlur || mPostProcessingOptions.mEdgeDetection)
                    ImGui::SliderFloat("Kernel offset", &mPostProcessingOptions.mKernelOffset, -1.f, 1.f);

                ImGui::TreePop();
            }
            ImGui::Separator();
        }
        ImGui::End();
    }

    void OpenGLRenderer::initialiseMesh(const Component::Mesh& pMesh, GLMeshData* pParentMesh /*= nullptr*/)
    {
        ZEPHYR_ASSERT(pParentMesh || mGLMeshData.size() == (pMesh.mID.Get()), "mGLMeshData size does not match Mesh ID. Has the order of Meshes changed or are they not ordered by MeshID");

        GLMeshData* newMesh = nullptr;
        if (pParentMesh)
        {
            pParentMesh->mChildMeshes.push_back({});
            newMesh = &pParentMesh->mChildMeshes.back();
        }
        else
        {
            mGLMeshData.push_back({});
            newMesh = &mGLMeshData.back();
        }

        if (pMesh.mName == "Quad")
            mScreenQuadMeshIndex = mGLMeshData.size() - 1;
        else if (pMesh.mName == "Skybox")
            mSkyBoxMeshIndex = mGLMeshData.size() - 1;
        else if (pMesh.mName == "3DCube")
            m3DCubeMeshIndex = mGLMeshData.size() - 1;

        newMesh->mDrawMode = GLType::PrimitiveMode::Triangles; // OpenGLRenderer only supports Triangles at this revision

        if (!pMesh.mIndices.empty())
        {
            newMesh->mDrawMethod = GLMeshData::DrawMethod::Indices;
            newMesh->mDrawSize   = static_cast<int>(pMesh.mIndices.size());
        }
        else
        {
            newMesh->mDrawMethod = GLMeshData::DrawMethod::Array;
            ZEPHYR_ASSERT(newMesh->mDrawMode == GLType::PrimitiveMode::Triangles, "Only PrimitiveMode::Triangles is supported");
            newMesh->mDrawSize = static_cast<int>(pMesh.mVertices.size()) / 3; // Divide verts by 3 as we draw the vertices by Triangles count.
        }

        newMesh->mVAO.generate();
        newMesh->mVAO.bind(); // Have to bind VAO before buffering VBO and EBO data

        if (!pMesh.mIndices.empty())
        {
            newMesh->mEBO.emplace(GLData::EBO(mGLState, GLType::BufferUsage::StaticDraw));
            newMesh->mEBO->Bind(mGLState);
            newMesh->mEBO->PushData(mGLState, pMesh.mIndices);
        }

        if (!pMesh.mVertices.empty())
        {
            newMesh->mVBOs[Utility::toIndex(Shader::Attribute::Position3D)].emplace(GLData::VBO(mGLState, GLType::BufferUsage::StaticDraw));
            newMesh->mVBOs[Utility::toIndex(Shader::Attribute::Position3D)]->Bind(mGLState);
            newMesh->mVBOs[Utility::toIndex(Shader::Attribute::Position3D)]->PushVertexAttributeData(mGLState, pMesh.mVertices, Shader::getAttributeLocation(Shader::Attribute::Position3D), Shader::getAttributeComponentCount(Shader::Attribute::Position3D));
        }
        if (!pMesh.mNormals.empty())
        {
            newMesh->mVBOs[Utility::toIndex(Shader::Attribute::Normal3D)].emplace(GLData::VBO(mGLState, GLType::BufferUsage::StaticDraw));
            newMesh->mVBOs[Utility::toIndex(Shader::Attribute::Normal3D)]->Bind(mGLState);
            newMesh->mVBOs[Utility::toIndex(Shader::Attribute::Normal3D)]->PushVertexAttributeData(mGLState, pMesh.mNormals, Shader::getAttributeLocation(Shader::Attribute::Normal3D), Shader::getAttributeComponentCount(Shader::Attribute::Normal3D));
        }
        if (!pMesh.mColours.empty())
        {
            newMesh->mVBOs[Utility::toIndex(Shader::Attribute::ColourRGB)].emplace(GLData::VBO(mGLState, GLType::BufferUsage::StaticDraw));
            newMesh->mVBOs[Utility::toIndex(Shader::Attribute::ColourRGB)]->Bind(mGLState);
            newMesh->mVBOs[Utility::toIndex(Shader::Attribute::ColourRGB)]->PushVertexAttributeData(mGLState, pMesh.mColours, Shader::getAttributeLocation(Shader::Attribute::ColourRGB), Shader::getAttributeComponentCount(Shader::Attribute::ColourRGB));
        }
        if (!pMesh.mTextureCoordinates.empty())
        {
            newMesh->mVBOs[Utility::toIndex(Shader::Attribute::TextureCoordinate2D)].emplace(GLData::VBO(mGLState, GLType::BufferUsage::StaticDraw));
            newMesh->mVBOs[Utility::toIndex(Shader::Attribute::TextureCoordinate2D)]->Bind(mGLState);
            newMesh->mVBOs[Utility::toIndex(Shader::Attribute::TextureCoordinate2D)]->PushVertexAttributeData(mGLState, pMesh.mTextureCoordinates, Shader::getAttributeLocation(Shader::Attribute::TextureCoordinate2D), Shader::getAttributeComponentCount(Shader::Attribute::TextureCoordinate2D));
        }

        // Recursively call initialiseMesh on all the child meshes appending them to GLMeshData::mChildMeshes.
        for (const auto& childMesh : pMesh.mChildMeshes)
            initialiseMesh(childMesh, newMesh);

        LOG_INFO("Component::Mesh: '{} (ID: {})' loaded into OpenGL with VAO: {}", pMesh.mName, pMesh.mID.Get(), newMesh->mVAO.getHandle());
    }

    const GLData::Texture& OpenGLRenderer::getTexture(const Component::TextureID& pTextureID) const
    {
        return mTextures[pTextureID.Get()];
    }

    void OpenGLRenderer::initialiseTexture(const Component::Texture& pTexture)
    {
        GLData::Texture newTexture = {GLData::Texture::Type::Texture2D};
        newTexture.generate();
        newTexture.bind();
        newTexture.pushData(pTexture.mWidth, pTexture.mHeight, pTexture.mNumberOfChannels, pTexture.getData());

        // Cache the ID of the 'missing' texture.
        if (pTexture.mName == "missing")
            mMissingTextureID = mTextures.size();

        ZEPHYR_ASSERT(mTextures.size() == pTexture.mID.Get(), "OpenGL::Texture does not match index position of Component::Texture::ID ({} != {})", mTextures.size(), pTexture.mID.Get());
        mTextures.push_back({newTexture});
        LOG_INFO("Component::Texture: '{} (ID: {})' loaded into OpenGL with VAO: {}", pTexture.mName, pTexture.mID.Get(), newTexture.getHandle());
    }

    void OpenGLRenderer::initialiseCubeMap(const Component::CubeMapTexture& pCubeMap)
    {
        // OpenGL cubeMap texture objects store all 6 faces under 1 VAO hence only one generate and bind is used before 6 pushData calls
        // Each face can be offset by index (last param of pushData) in the order Right (0), Left (1), Top(2), Bottom(3), Back(4), Front(5)
        GLData::Texture newCubeMap = {GLData::Texture::Type::CubeMap};
        newCubeMap.generate();
        newCubeMap.bind();

        newCubeMap.pushData(pCubeMap.mRight.mWidth, pCubeMap.mRight.mHeight, pCubeMap.mRight.mNumberOfChannels, pCubeMap.mRight.getData(), 0);
        newCubeMap.pushData(pCubeMap.mLeft.mWidth, pCubeMap.mLeft.mHeight, pCubeMap.mLeft.mNumberOfChannels, pCubeMap.mLeft.getData(), 1);
        newCubeMap.pushData(pCubeMap.mTop.mWidth, pCubeMap.mTop.mHeight, pCubeMap.mTop.mNumberOfChannels, pCubeMap.mTop.getData(), 2);
        newCubeMap.pushData(pCubeMap.mBottom.mWidth, pCubeMap.mBottom.mHeight, pCubeMap.mBottom.mNumberOfChannels, pCubeMap.mBottom.getData(), 3);
        newCubeMap.pushData(pCubeMap.mFront.mWidth, pCubeMap.mFront.mHeight, pCubeMap.mFront.mNumberOfChannels, pCubeMap.mFront.getData(), 4);
        newCubeMap.pushData(pCubeMap.mBack.mWidth, pCubeMap.mBack.mHeight, pCubeMap.mBack.mNumberOfChannels, pCubeMap.mBack.getData(), 5);

        mCubeMaps.push_back(newCubeMap);
        LOG_INFO("Component::CubeMapTexture: '{}' loaded into OpenGL with VAO: {}", pCubeMap.mName, newCubeMap.getHandle());
    }

    GladGLContext* OpenGLRenderer::initialiseGLAD()
    {
        GladGLContext* GLADContext = (GladGLContext*)malloc(sizeof(GladGLContext));
        int version                = gladLoadGLContext(GLADContext, glfwGetProcAddress);
        ZEPHYR_ASSERT(GLADContext && version != 0, "Failed to initialise GLAD GL context");
        // TODO: Add an assert here for GLAD_VERSION to equal to cOpenGLVersion
        LOG_INFO("Initialised GLAD using OpenGL {}.{}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
        return GLADContext;
    }

    void OpenGLRenderer::onResize(const int pWidth, const int pHeight)
    {
        mMainScreenFBO.resize(pWidth, pHeight, mGLState);
        mGLState.setViewport(pWidth, pHeight);
        mWindow.mWidth       = pWidth;
        mWindow.mHeight      = pHeight;
        mWindow.mAspectRatio = static_cast<float>(pWidth) / static_cast<float>(pHeight);
    }

    void OpenGLRenderer::windowSizeCallback(GLFWwindow* pWindow, int pWidth, int pHeight)
    {
        const float width  = static_cast<float>(pWidth);
        const float height = static_cast<float>(pHeight);
        LOG_INFO("OpenGL Window resolution changed to {}x{}", pWidth, pHeight);

        ImGuiIO& io        = ImGui::GetIO();
        io.DisplaySize     = ImVec2(width, height);
        io.FontGlobalScale = std::round(ImGui::GetMainViewport()->DpiScale);

        for (auto* OpenGLinstance : OpenGLInstances)
        {
            if (OpenGLinstance)
            {
                OpenGLinstance->onResize(pWidth, pHeight);
            }
        }
    }
} // namespace OpenGL