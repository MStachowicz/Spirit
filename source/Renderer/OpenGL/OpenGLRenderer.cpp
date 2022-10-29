#include "OpenGLRenderer.hpp"

// ECS
#include "EntityManager.hpp"

// DATA
#include "DirectionalLight.hpp"
#include "PointLight.hpp"
#include "SpotLight.hpp"
#include "Texture.hpp"

// MANAGER
#include "Managers/MeshManager.hpp"
#include "Managers/TextureManager.hpp"
#include "Managers/CameraManager.hpp"

// External libs
#include "glad/gl.h"
#include "GLFW/glfw3.h" // Used to initialise GLAD using glfwGetProcAddress
#include "Logger.hpp"
#include "glm/ext/matrix_transform.hpp" // perspective, translate, rotate
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"

// STD
#include <algorithm>

namespace OpenGL
{
    OpenGLRenderer::OpenGLRenderer(ECS::EntityManager& pEntityManager, const Manager::MeshManager& pMeshManager, const Manager::TextureManager& pTextureManager, Manager::CameraManager& pCameraManager)
        : cOpenGLVersionMajor(4)
        , cOpenGLVersionMinor(3)
        , mLinearDepthView(false)
        , mVisualiseNormals(false)
        , mUseInstancedDraw(false)
        , mInstancingCountThreshold(20)
        , mZNearPlane(0.1f)
        , mZFarPlane(100.0f)
        , mFOV(45.f)
        , mWindow(cOpenGLVersionMajor, cOpenGLVersionMinor)
        , mGLADContext(initialiseGLAD()) // TODO: This should only happen on first OpenGLRenderer construction (OpenGLInstances.size() == 1)
        , mGLState()
        , mTexture1ShaderIndex(0)
        , mTexture2ShaderIndex(1)
        , mMaterialShaderIndex(2)
        , mUniformShaderIndex(4)
        , mLightMapIndex(5)
        , mTexture1InstancedShaderIndex(6)
        , mMissingTextureID()
        , pointLightDrawCount(0)
        , spotLightDrawCount(0)
        , directionalLightDrawCount(0)
        , mBufferDrawType(BufferDrawType::Colour)
        , mPostProcessingOptions()
        , mScreenQuad()
        , mScreenTextureShader("screenTexture", mGLState)
        , mSkyBoxMeshID()
        , mSkyBoxShader("skybox", mGLState)
        , m3DCubeID()
        , mLightEmitterShader("uniformColour", mGLState)
        , mDepthViewerShader("depthView", mGLState)
        , mVisualiseNormalShader("visualiseNormal", mGLState)
        , mAvailableShaders{Shader("texture1", mGLState), Shader("texture2", mGLState), Shader("material", mGLState), Shader("colour", mGLState), Shader("uniformColour", mGLState), Shader("lightMap", mGLState), Shader("texture1Instanced", mGLState)}
        , mEntityManager(pEntityManager)
    {
        pMeshManager.ForEach([this](const auto& mesh) { initialiseMesh(mesh); }); // Depends on mShaders being initialised.
        pTextureManager.ForEach([this](const auto& texture) { initialiseTexture(texture); });
        pTextureManager.ForEachCubeMap([this](const auto& cubeMap) { initialiseCubeMap(cubeMap); });

        pEntityManager.mEntityCreatedEvent.Subscribe(std::bind(&OpenGL::OpenGLRenderer::onEntityCreated, this, std::placeholders::_1, std::placeholders::_2));
        pEntityManager.mEntityRemovedEvent.Subscribe(std::bind(&OpenGL::OpenGLRenderer::onEntityRemoved, this, std::placeholders::_1, std::placeholders::_2));

        pEntityManager.mTransforms.mComponentAddedEvent.Subscribe(std::bind(&OpenGL::OpenGLRenderer::onTransformComponentAdded, this, std::placeholders::_1, std::placeholders::_2));
        pEntityManager.mTransforms.mComponentChangedEvent.Subscribe(std::bind(&OpenGL::OpenGLRenderer::onTransformComponentChanged, this, std::placeholders::_1, std::placeholders::_2));
        pEntityManager.mTransforms.mComponentRemovedEvent.Subscribe(std::bind(&OpenGL::OpenGLRenderer::onTransformComponentRemoved, this, std::placeholders::_1));

        pEntityManager.mMeshes.mComponentAddedEvent.Subscribe(std::bind(&OpenGL::OpenGLRenderer::onMeshComponentAdded, this, std::placeholders::_1, std::placeholders::_2));
        // pEntityManager.mMeshes.mComponentChangedEvent.Subscribe(std::bind(&OpenGL::OpenGLRenderer::onMeshComponentChanged, this, std::placeholders::_1, std::placeholders::_2));
        pEntityManager.mMeshes.mComponentRemovedEvent.Subscribe(std::bind(&OpenGL::OpenGLRenderer::onMeshComponentRemoved, this, std::placeholders::_1));

        pCameraManager.mPrimaryCameraViewChanged.Subscribe(std::bind(&OpenGL::OpenGLRenderer::setView, this, std::placeholders::_1));
        pCameraManager.mPrimaryCameraViewPositionChanged.Subscribe(std::bind(&OpenGL::OpenGLRenderer::setViewPosition, this, std::placeholders::_1));
        const auto primaryCam = pCameraManager.getPrimaryCamera();
        setView(primaryCam.getViewMatrix());
        setViewPosition(primaryCam.getPosition());

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

    void OpenGLRenderer::removeEntityDrawCall(const ECS::Entity& pEntity)
    {
        // Find the DrawCall containing pEntity data and remove it.
        // Move the last model into the to-be-deleted position and pop the back.
        // Finally, update the mEntityModelIndexLookup of the moved entity data to point to the position of the removed data.
        for (size_t i = 0; i < mDrawCalls.size(); i++)
        {
            auto entityIndexToRemove = mDrawCalls[i].mEntityModelIndexLookup.find(pEntity.mID);
            if (entityIndexToRemove != mDrawCalls[i].mEntityModelIndexLookup.end())
            {
                const size_t removedEntityModelsIndex = entityIndexToRemove->second;
                mDrawCalls[i].mEntityModelIndexLookup.erase(entityIndexToRemove);

                if (removedEntityModelsIndex < mDrawCalls[i].mModels.size())
                {
                    mDrawCalls[i].mModels[removedEntityModelsIndex] = std::move(mDrawCalls[i].mModels.back());

                    // Reverse find the moved entity in mEntityModelIndexLookup by searching the map for the final index.
                    // If this becomes a performance problem, on add, we can cache the last element EntityID and perform a
                    // regular map search.
                    const size_t movedEntityIndex = mDrawCalls[i].mModels.size() - 1;
                    for (auto& entityIndex : mDrawCalls[i].mEntityModelIndexLookup)
                    {
                        if (entityIndex.second == movedEntityIndex)
                        {
                            entityIndex.second = removedEntityModelsIndex;
                            break;
                        }
                    }
                }

                mDrawCalls[i].mModels.pop_back();
                return;
            }
        }
    }

    void OpenGLRenderer::addEntityDrawCall(const ECS::Entity& pEntity, const Data::Transform& pTransform, const Data::MeshDraw& pMesh)
    {
        // If an entity has a MeshDraw and Transform component, add it to the mDrawCalls list.
        // If the MeshDraw variation already exists in a DrawCall, append just the Transform data to the mModels.
        // We store a copy of MeshDraw in a single DrawCall so while the data is copied here, it only exists once for each unique mesh.

        auto drawCallIt = std::find_if(mDrawCalls.begin(), mDrawCalls.end(), [pMesh](const DrawCall& entry)
                                       { return entry.mMesh.mID == pMesh.mID && entry.mMesh.mDrawMode == pMesh.mDrawMode && entry.mMesh.mDrawStyle == pMesh.mDrawStyle
                                                // Per DrawStyle values
                                                && entry.mMesh.mTexture1 == pMesh.mTexture1 && entry.mMesh.mTexture2 == pMesh.mTexture2 && entry.mMesh.mMixFactor == pMesh.mMixFactor && entry.mMesh.mColour == pMesh.mColour && entry.mMesh.mDiffuseTextureID == pMesh.mDiffuseTextureID && entry.mMesh.mSpecularTextureID == pMesh.mSpecularTextureID && entry.mMesh.mShininess == pMesh.mShininess && entry.mMesh.mTextureRepeatFactor == pMesh.mTextureRepeatFactor; });

        if (drawCallIt == mDrawCalls.end())
        {
            DrawCall drawCall;
            drawCall.mMesh = pMesh;
            mDrawCalls.push_back(drawCall);
            mDrawCallToShader.push_back(std::nullopt);
            drawCallIt = mDrawCalls.end() - 1;
        }

        drawCallIt->mEntityModelIndexLookup[pEntity.mID] = drawCallIt->mModels.size();
        drawCallIt->mModels.push_back(util::GetModelMatrix(pTransform.mPosition, pTransform.mRotation, pTransform.mScale));
        const bool updated = updateShader(*drawCallIt, drawCallIt - mDrawCalls.begin());

        if (!updated)
        {
            // If updateShader doesnt assign a new shader to the DrawCall, it does not update the buffer data, we manually update the buffer for the
            // newly pushed back model matrix of Transform component.
            auto shader = getShader(*drawCallIt, drawCallIt - mDrawCalls.begin());
            if (shader->isInstanced())
            {
                auto instanceModelsArray = shader->getShaderBlockVariable("InstancedData.models[0]");
                instanceModelsArray->Set(mGLState, drawCallIt->mModels.back(), drawCallIt->mModels.size() - 1);
            }
        }
    }

    void OpenGLRenderer::onEntityCreated(const ECS::Entity& pEntity, const ECS::EntityManager& pManager)
    {
        if (const Data::MeshDraw* mesh = pManager.mMeshes.GetComponent(pEntity))
            if (const Data::Transform* transform = pManager.mTransforms.GetComponent(pEntity))
                addEntityDrawCall(pEntity, *transform, *mesh);
    }

    void OpenGLRenderer::onEntityRemoved(const ECS::Entity& pEntity, const ECS::EntityManager& pManager)
    {
        removeEntityDrawCall(pEntity);
    }
    void OpenGLRenderer::onTransformComponentRemoved(const ECS::Entity& pEntity)
    {
        removeEntityDrawCall(pEntity);
    }
    void OpenGLRenderer::onMeshComponentRemoved(const ECS::Entity& pEntity)
    {
        removeEntityDrawCall(pEntity);
    }

    void OpenGLRenderer::onTransformComponentAdded(const ECS::Entity& pEntity, const Data::Transform& pTransform)
    {
        if (const auto mesh = mEntityManager.mMeshes.GetComponent(pEntity))
            addEntityDrawCall(pEntity, pTransform, *mesh);
    }
    void OpenGLRenderer::onTransformComponentChanged(const ECS::Entity& pEntity, const Data::Transform& pTransform)
    {
        // Find the DrawCall containing pEntity Transform data and update the model matrix for it.
        for (size_t i = 0; i < mDrawCalls.size(); i++)
        {
            auto it = mDrawCalls[i].mEntityModelIndexLookup.find(pEntity.mID);
            if (it != mDrawCalls[i].mEntityModelIndexLookup.end())
            {
                mDrawCalls[i].mModels[it->second] = util::GetModelMatrix(pTransform.mPosition, pTransform.mRotation, pTransform.mScale);

                // If the shader is instanced, we have to update the instance data array directly at the corresponding index.
                auto shader = getShader(mDrawCalls[i], i);
                if (shader->isInstanced())
                {
                    auto instanceModelsArray = shader->getShaderBlockVariable("InstancedData.models[0]");
                    instanceModelsArray->Set(mGLState, mDrawCalls[i].mModels[it->second], it->second);
                }
                return;
            }
        }
    }

    void OpenGLRenderer::onMeshComponentAdded(const ECS::Entity& pEntity, const Data::MeshDraw& pMesh)
    {
        if (const auto transform = mEntityManager.mTransforms.GetComponent(pEntity))
            addEntityDrawCall(pEntity, *transform, pMesh);
    }

    bool OpenGLRenderer::updateShader(const DrawCall& pDrawCall, const size_t& pDrawCallIndex)
    {
        ZEPHYR_ASSERT(mDrawCallToShader.size() == mDrawCalls.size(), "DrawCall to shader mapping must remain 1-1. Was a DrawCall added or removed but not had a shader set?");

        Shader* shaderToUse = nullptr;
        switch (pDrawCall.mMesh.mDrawStyle)
        {
            case Data::DrawStyle::Textured:
                if (pDrawCall.mMesh.mTexture1.has_value() && pDrawCall.mMesh.mTexture2.has_value())
                    shaderToUse = &mAvailableShaders[mTexture2ShaderIndex];
                else
                    shaderToUse = &mAvailableShaders[mTexture1ShaderIndex];
                break;
            case Data::DrawStyle::UniformColour:
                shaderToUse = &mAvailableShaders[mUniformShaderIndex];
                break;
            case Data::DrawStyle::LightMap:
                shaderToUse = &mAvailableShaders[mLightMapIndex];
                break;
        }
        ZEPHYR_ASSERT(shaderToUse != nullptr, "Couldn't identify a shader to render this DrawCall.");

        if (mUseInstancedDraw && pDrawCall.mModels.size() >= mInstancingCountThreshold)
            if (auto* shader = getInstancedShader(*shaderToUse))
                shaderToUse = shader;
            else
                LOG_INFO("DrawCall reached the instanced threshold but no instanced shader was present to use. Add an instanced version of Shader '{}'", shaderToUse->getName());

        if (mDrawCallToShader[pDrawCallIndex].has_value() && mDrawCallToShader[pDrawCallIndex].value().getName() == shaderToUse->getName())
            return false; // Already using the correct shader.
        else
        {
            // Assign the new shader
            mDrawCallToShader[pDrawCallIndex] = Shader(shaderToUse->getName(), mGLState);

            // If the newly assigned shader is an instanced one, update all the model data.
            if (mDrawCallToShader[pDrawCallIndex]->isInstanced())
            {
                auto instanceModelsArray = mDrawCallToShader[pDrawCallIndex]->getShaderBlockVariable("InstancedData.models[0]");
                for (size_t i = 0; i < pDrawCall.mModels.size(); i++)
                    instanceModelsArray->Set(mGLState, pDrawCall.mModels[i], i);
            }
            return true;
        }
    }

    void OpenGLRenderer::onInstancedOptionChanged()
    {
        for (size_t i = 0; i < mDrawCalls.size(); i++)
            updateShader(mDrawCalls[i], i);
    }

    void OpenGLRenderer::preDraw()
    {
        mMainScreenFBO.bind(mGLState);
        mMainScreenFBO.clearBuffers();
        mGLState.checkFramebufferBufferComplete();

        // #OPTIMISATION do this only when view or projection changes.
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

        // TODO: Set this for all shaders that use viewPosition + make this more generalised (find any shaders with viewPosition vars to set?).
        for (size_t i = 0; i < mDrawCallToShader.size(); i++)
        {
            if (mDrawCallToShader[i].has_value() && mDrawCallToShader[i]->getName() == "lightMap")
            {
                mDrawCallToShader[i]->use(mGLState);
                mDrawCallToShader[i]->setUniform(mGLState, "viewPosition", mViewPosition);
            }
        }
    }

    Shader* OpenGLRenderer::getShader(const DrawCall& pDrawCall, const size_t& pDrawCallIndex)
    {
        ZEPHYR_ASSERT(pDrawCallIndex < mDrawCallToShader.size() && mDrawCallToShader[pDrawCallIndex].has_value(), "Could not find a shader to execute this DrawCall with");
        return &mDrawCallToShader[pDrawCallIndex].value();
    }

    Shader* OpenGLRenderer::getInstancedShader(const Shader& pShader)
    {
        ZEPHYR_ASSERT(!pShader.isInstanced(), "Trying to find an instanced version of an already instanced shader.")

        for (size_t i = 0; i < mAvailableShaders.size(); i++)
        {
            if (mAvailableShaders[i].getName() == pShader.getName() + "Instanced")
                return &mAvailableShaders[i];
        }
        return nullptr;
    }

    void OpenGLRenderer::draw()
    {
        for (size_t i = 0; i < mDrawCalls.size(); i++)
        {
            if (mDrawCalls[i].mModels.empty())
                continue;

            const OpenGLMesh& GLMesh = getGLMesh(mDrawCalls[i].mMesh.mID);
            if (Shader* shader = mBufferDrawType == BufferDrawType::Depth ? &mDepthViewerShader : getShader(mDrawCalls[i], i))
            {
                shader->use(mGLState);

                if (shader->getName() == "texture1" || shader->getName() == "texture1Instanced")
                {
                    ZEPHYR_ASSERT(mDrawCalls[i].mMesh.mTexture1.has_value(), "DrawCall must have mTexture1 set to draw using texture1 shader");
                    mGLState.setActiveTextureUnit(0);
                    getTexture(mDrawCalls[i].mMesh.mTexture1.value()).bind();
                }
                else if (shader->getName() == "texture2")
                {
                    ZEPHYR_ASSERT(mDrawCalls[i].mMesh.mMixFactor.has_value(), "DrawCall must have mixFactor set to draw using texture2 shader");
                    ZEPHYR_ASSERT(mDrawCalls[i].mMesh.mTexture1.has_value(), "DrawCall must have mTexture1 set to draw using texture2 shader");
                    ZEPHYR_ASSERT(mDrawCalls[i].mMesh.mTexture2.has_value(), "DrawCall must have mTexture2 set to draw using texture2 shader");

                    shader->setUniform(mGLState, "mixFactor", mDrawCalls[i].mMesh.mMixFactor.value());
                    mGLState.setActiveTextureUnit(0);
                    getTexture(mDrawCalls[i].mMesh.mTexture1.value()).bind();
                    mGLState.setActiveTextureUnit(1);
                    getTexture(mDrawCalls[i].mMesh.mTexture2.value()).bind();
                }
                else if (shader->getName() == "uniformColour")
                {
                    ZEPHYR_ASSERT(mDrawCalls[i].mMesh.mColour.has_value(), "DrawCall must have mColour set to draw using uniformColour shader");
                    shader->setUniform(mGLState, "colour", mDrawCalls[i].mMesh.mColour.value());
                }
                else if (shader->getName() == "lightMap")
                {
                    ZEPHYR_ASSERT(GLMesh.mDrawSize == 0 || GLMesh.mVBOs[util::toIndex(Shader::Attribute::Normal3D)].has_value(), "Cannot draw a mesh with no Normal data using lightMap shader.")
                    ZEPHYR_ASSERT(mDrawCalls[i].mMesh.mDiffuseTextureID.has_value(), "DrawCall must have mDiffuseTextureID set to draw using lightMap shader");
                    ZEPHYR_ASSERT(mDrawCalls[i].mMesh.mSpecularTextureID.has_value(), "DrawCall must have mSpecularTextureID set to draw using lightMap shader");
                    ZEPHYR_ASSERT(mDrawCalls[i].mMesh.mShininess.has_value(), "DrawCall must have mTexture2 set to draw using texture2");

                    mGLState.setActiveTextureUnit(0);
                    getTexture(mDrawCalls[i].mMesh.mDiffuseTextureID.value()).bind();
                    mGLState.setActiveTextureUnit(1);
                    getTexture(mDrawCalls[i].mMesh.mSpecularTextureID.value()).bind();
                    shader->setUniform(mGLState, "shininess", mDrawCalls[i].mMesh.mShininess.value());
                    shader->setUniform(mGLState, "textureRepeatFactor", mDrawCalls[i].mMesh.mTextureRepeatFactor.has_value() ? mDrawCalls[i].mMesh.mTextureRepeatFactor.value() : 1.f);
                }
                else if (shader->getName() == "depthView")
                {
                }
                else
                    ZEPHYR_ASSERT(false, "Shader {} not found for setting uniform variables. Do you need to add a new shader to the above list?");

                switch (mDrawCalls[i].mMesh.mDrawMode)
                {
                    case Data::DrawMode::Fill: mGLState.setPolygonMode(GLType::PolygonMode::Fill); break;
                    case Data::DrawMode::Wireframe: mGLState.setPolygonMode(GLType::PolygonMode::Line); break;
                    default: ZEPHYR_ASSERT(false, "Unknown drawMode requested for OpenGLRenderer draw."); break;
                }

                // Instanced shaders set their models in buffers so dont need to set the model matrix here, just call draw.
                if (shader->isInstanced())
                {
                    draw(GLMesh, mDrawCalls[i].mModels.size());
                }
                else
                {
                    for (const auto& model : mDrawCalls[i].mModels)
                    {
                        shader->setUniform(mGLState, "model", model);
                        draw(GLMesh);
                        if (mVisualiseNormals)
                        {
                            mVisualiseNormalShader.setUniform(mGLState, "model", model);
                            draw(GLMesh);
                        }
                    }
                }
            }
        }
    }

    void OpenGLRenderer::draw(const OpenGLRenderer::OpenGLMesh& pMesh, const size_t& pInstancedCount /* = 0*/)
    {
        if (pMesh.mDrawSize > 0)
        {
            pMesh.mVAO.bind();

            if (pInstancedCount > 0)
            {
                if (pMesh.mDrawMethod == OpenGLMesh::DrawMethod::Indices)
                    mGLState.drawElementsInstanced(pMesh.mDrawMode, pMesh.mDrawSize, static_cast<int>(pInstancedCount));
                else if (pMesh.mDrawMethod == OpenGLMesh::DrawMethod::Array)
                    mGLState.drawArraysInstanced(pMesh.mDrawMode, pMesh.mDrawSize, static_cast<int>(pInstancedCount));
            }
            else
            {
                if (pMesh.mDrawMethod == OpenGLMesh::DrawMethod::Indices)
                    mGLState.drawElements(pMesh.mDrawMode, pMesh.mDrawSize);
                else if (pMesh.mDrawMethod == OpenGLMesh::DrawMethod::Array)
                    mGLState.drawArrays(pMesh.mDrawMode, pMesh.mDrawSize);
            }
        }

        for (const auto& childMesh : pMesh.mChildMeshes)
            draw(childMesh);
    }

    void OpenGLRenderer::setupLights(const bool& pRenderLightPositions)
    {
        mEntityManager.mPointLights.ForEach([this](const Data::PointLight& pPointLight)
                                            { setShaderVariables(pPointLight); });
        mEntityManager.mDirectionalLights.ForEach([this](const Data::DirectionalLight& pDirectionalLight)
                                                  { setShaderVariables(pDirectionalLight); });
        mEntityManager.mSpotLights.ForEach([this](const Data::SpotLight& pSpotLight)
                                           { setShaderVariables(pSpotLight); });

        if (pRenderLightPositions)
        {
            mLightEmitterShader.use(mGLState);
            mEntityManager.mPointLights.ForEach([this](const Data::PointLight& pPointLight)
                                                {
			mLightEmitterShader.setUniform(mGLState, "model", util::GetModelMatrix(pPointLight.mPosition, glm::vec3(0.f), glm::vec3(0.1f)));
			mLightEmitterShader.setUniform(mGLState, "colour", pPointLight.mColour);
   			draw(getGLMesh(m3DCubeID)); });

            mLightEmitterShader.use(mGLState);
            mGLState.setPolygonMode(GLType::PolygonMode::Line);
            mEntityManager.mColliders.ForEach([this](const Data::Collider& pCollider)
                                              {
			const auto highPoint = glm::vec3(pCollider.mBoundingBox.mHighX, pCollider.mBoundingBox.mHighY, pCollider.mBoundingBox.mHighZ);
			const auto lowPoint = glm::vec3(pCollider.mBoundingBox.mLowX, pCollider.mBoundingBox.mLowY, pCollider.mBoundingBox.mLowZ);
			const auto lowToHigh = highPoint - lowPoint;
			const glm::vec3 center = lowPoint + (lowToHigh / 2.f);
			const auto sizeX = pCollider.mBoundingBox.mHighX - pCollider.mBoundingBox.mLowX;
			const auto sizeY = pCollider.mBoundingBox.mHighY - pCollider.mBoundingBox.mLowY;
			const auto sizeZ = pCollider.mBoundingBox.mHighZ - pCollider.mBoundingBox.mLowZ;

			mLightEmitterShader.setUniform(mGLState, "model", util::GetModelMatrix(center, glm::vec3(0.f), glm::vec3(sizeX, sizeY, sizeZ)));
   			mLightEmitterShader.setUniform(mGLState, "colour", glm::vec3(0.f, 1.f, 0.f));
   			draw(getGLMesh(m3DCubeID)); });
            mGLState.setPolygonMode(GLType::PolygonMode::Fill);
        }
    }

    void OpenGLRenderer::setShaderVariables(const Data::PointLight& pPointLight)
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

    void OpenGLRenderer::setShaderVariables(const Data::DirectionalLight& pDirectionalLight)
    {
        const glm::vec3 diffuseColour = pDirectionalLight.mColour * pDirectionalLight.mDiffuseIntensity;
        const glm::vec3 ambientColour = diffuseColour * pDirectionalLight.mAmbientIntensity;

        mGLState.setUniformBlockVariable("Lights.mDirectionalLight.direction", pDirectionalLight.mDirection);
        mGLState.setUniformBlockVariable("Lights.mDirectionalLight.ambient", ambientColour);
        mGLState.setUniformBlockVariable("Lights.mDirectionalLight.diffuse", diffuseColour);
        mGLState.setUniformBlockVariable("Lights.mDirectionalLight.specular", glm::vec3(pDirectionalLight.mSpecularIntensity));

        directionalLightDrawCount++;
    }
    void OpenGLRenderer::setShaderVariables(const Data::SpotLight& pSpotLight)
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
            draw(getGLMesh(mSkyBoxMeshID));

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
            draw(getGLMesh(mScreenQuad));

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

            static const std::array<std::string, util::toIndex(BufferDrawType::Count)> bufferDrawTypes{
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

            {
                ImGui::Separator(); // Instancing options
                if (ImGui::Checkbox("Use instanced rendering", &mUseInstancedDraw))
                    onInstancedOptionChanged();
                if (mUseInstancedDraw)
                    if (ImGui::SliderInt("Instanced rendering threshold", &mInstancingCountThreshold, 1, 1000))
                        onInstancedOptionChanged();
            }
            ImGui::Separator();

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

    const OpenGLRenderer::OpenGLMesh& OpenGLRenderer::getGLMesh(const MeshID& pMeshID) const
    {
        const auto it = std::find_if(mGLMeshes.begin(), mGLMeshes.end(), [&pMeshID](const OpenGLMesh& pGLMesh)
                                     { return pMeshID.Get() == pGLMesh.mID.Get(); });

        ZEPHYR_ASSERT(it != mGLMeshes.end(), "No matching OpenGL::Mesh found for Data::Mesh with ID '{}'. Was the mesh correctly initialised?", pMeshID.Get());
        return *it;
    }

    void OpenGLRenderer::initialiseMesh(const Data::Mesh& pMesh)
    {
        OpenGLMesh* newMesh = nullptr;
        {
            auto GLMeshLocation = std::find_if(mGLMeshes.begin(), mGLMeshes.end(), [&pMesh](const OpenGLMesh& pGLMesh)
                                               { return pMesh.mID.Get() == pGLMesh.mID.Get(); });

            if (GLMeshLocation != mGLMeshes.end())
            {
                GLMeshLocation->mChildMeshes.push_back({});
                newMesh = &GLMeshLocation->mChildMeshes.back();
            }
            else
            {
                mGLMeshes.push_back({});
                newMesh = &mGLMeshes.back();

                if (pMesh.mName == "Quad")
                    mScreenQuad = pMesh.mID;
                else if (pMesh.mName == "Skybox")
                    mSkyBoxMeshID = pMesh.mID;
                else if (pMesh.mName == "3DCube")
                    m3DCubeID = pMesh.mID;
            }
        }
        ZEPHYR_ASSERT(newMesh != nullptr, "Failed to initialise Data::Mesh with ID '{}'", pMesh.mID.Get());

        newMesh->mID       = pMesh.mID;
        newMesh->mDrawMode = GLType::PrimitiveMode::Triangles; // OpenGLRenderer only supports Triangles at this revision

        if (!pMesh.mIndices.empty())
        {
            newMesh->mDrawMethod = OpenGLMesh::DrawMethod::Indices;
            newMesh->mDrawSize   = static_cast<int>(pMesh.mIndices.size());
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
            newMesh->mEBO.emplace(GLData::EBO(mGLState, GLType::BufferUsage::StaticDraw));
            newMesh->mEBO->Bind(mGLState);
            newMesh->mEBO->PushData(mGLState, pMesh.mIndices);
        }

        if (!pMesh.mVertices.empty())
        {
            newMesh->mVBOs[util::toIndex(Shader::Attribute::Position3D)].emplace(GLData::VBO(mGLState, GLType::BufferUsage::StaticDraw));
            newMesh->mVBOs[util::toIndex(Shader::Attribute::Position3D)]->Bind(mGLState);
            newMesh->mVBOs[util::toIndex(Shader::Attribute::Position3D)]->PushVertexAttributeData(mGLState, pMesh.mVertices, Shader::getAttributeLocation(Shader::Attribute::Position3D), Shader::getAttributeComponentCount(Shader::Attribute::Position3D));
        }
        if (!pMesh.mNormals.empty())
        {
            newMesh->mVBOs[util::toIndex(Shader::Attribute::Normal3D)].emplace(GLData::VBO(mGLState, GLType::BufferUsage::StaticDraw));
            newMesh->mVBOs[util::toIndex(Shader::Attribute::Normal3D)]->Bind(mGLState);
            newMesh->mVBOs[util::toIndex(Shader::Attribute::Normal3D)]->PushVertexAttributeData(mGLState, pMesh.mNormals, Shader::getAttributeLocation(Shader::Attribute::Normal3D), Shader::getAttributeComponentCount(Shader::Attribute::Normal3D));
        }
        if (!pMesh.mColours.empty())
        {
            newMesh->mVBOs[util::toIndex(Shader::Attribute::ColourRGB)].emplace(GLData::VBO(mGLState, GLType::BufferUsage::StaticDraw));
            newMesh->mVBOs[util::toIndex(Shader::Attribute::ColourRGB)]->Bind(mGLState);
            newMesh->mVBOs[util::toIndex(Shader::Attribute::ColourRGB)]->PushVertexAttributeData(mGLState, pMesh.mColours, Shader::getAttributeLocation(Shader::Attribute::ColourRGB), Shader::getAttributeComponentCount(Shader::Attribute::ColourRGB));
        }
        if (!pMesh.mTextureCoordinates.empty())
        {
            newMesh->mVBOs[util::toIndex(Shader::Attribute::TextureCoordinate2D)].emplace(GLData::VBO(mGLState, GLType::BufferUsage::StaticDraw));
            newMesh->mVBOs[util::toIndex(Shader::Attribute::TextureCoordinate2D)]->Bind(mGLState);
            newMesh->mVBOs[util::toIndex(Shader::Attribute::TextureCoordinate2D)]->PushVertexAttributeData(mGLState, pMesh.mTextureCoordinates, Shader::getAttributeLocation(Shader::Attribute::TextureCoordinate2D), Shader::getAttributeComponentCount(Shader::Attribute::TextureCoordinate2D));
        }

        for (const auto& childMesh : pMesh.mChildMeshes)
            initialiseMesh(childMesh);

        ZEPHYR_ASSERT(mGLMeshes.size() == (newMesh->mID.Get() + 1), "OpenGL::Mesh::ID {} does not match index position in Mesh container.", newMesh->mID.Get());
        ZEPHYR_ASSERT(pMesh.mID.Get() == newMesh->mID.Get(), "MeshID's do not match.");
        LOG_INFO("Data::Mesh: '{} (ID: {})' loaded into OpenGL with ID: '{}' and VAO: {}", pMesh.mName, pMesh.mID.Get(), newMesh->mID.Get(), newMesh->mVAO.getHandle());
    }

    const GLData::Texture& OpenGLRenderer::getTexture(const TextureID& pTextureID) const
    {
        return mTextures[pTextureID.Get()];
    }

    void OpenGLRenderer::initialiseTexture(const Data::Texture& pTexture)
    {
        GLData::Texture newTexture = {GLData::Texture::Type::Texture2D};
        newTexture.generate();
        newTexture.bind();
        newTexture.pushData(pTexture.mWidth, pTexture.mHeight, pTexture.mNumberOfChannels, pTexture.getData());

        // Cache the ID of the 'missing' texture.
        if (pTexture.mName == "missing")
            mMissingTextureID = pTexture.mID;

        ZEPHYR_ASSERT(mTextures.size() == pTexture.mID.Get(), "OpenGL::Texture does not match index position of Data::Texture::ID ({} != {})", mTextures.size(), pTexture.mID.Get());
        mTextures.push_back({newTexture});
        LOG_INFO("Data::Texture: '{} (ID: {})' loaded into OpenGL with VAO: {}", pTexture.mName, pTexture.mID.Get(), newTexture.getHandle());
    }

    void OpenGLRenderer::initialiseCubeMap(const Data::CubeMapTexture& pCubeMap)
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
        LOG_INFO("Data::CubeMapTexture: '{}' loaded into OpenGL with VAO: {}", pCubeMap.mName, newCubeMap.getHandle());
    }

    GladGLContext* OpenGLRenderer::initialiseGLAD()
    {
        GladGLContext* GLADContext = (GladGLContext*)malloc(sizeof(GladGLContext));
        int version                = gladLoadGLContext(GLADContext, glfwGetProcAddress);
        ZEPHYR_ASSERT(GLADContext && version != 0, "Failed to initialise GLAD GL context")
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