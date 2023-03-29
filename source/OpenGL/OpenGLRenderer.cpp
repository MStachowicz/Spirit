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
#include "SceneSystem.hpp"

// UTILITY
#include "Logger.hpp"
#include "Utility.hpp"

// PLATFORM
#include "Core.hpp"

//GLM
#include "glm/ext/matrix_transform.hpp" // perspective, translate, rotate
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/string_cast.hpp"

// STD
#include <algorithm>

#include "glad/gl.h"

namespace OpenGL
{
    OpenGLRenderer::DebugOptions::DebugOptions(GLState& pGLState)
        : mShowLightPositions{false}
        , mVisualiseNormals{false}
        , mForceClearColour{false}
        , mClearColour{0.f, 0.f, 0.f, 0.f}
        , mForceDepthTestType{false}
        , mForcedDepthTestType{GLType::DepthTestType::Less}
        , mForceBlendType{false}
        , mForcedSourceBlendType{GLType::BlendFactorType::SourceAlpha}
        , mForcedDestinationBlendType{GLType::BlendFactorType::OneMinusSourceAlpha}
        , mForceCullFacesType{false}
        , mForcedCullFacesType{GLType::CullFacesType::Back}
        , mForceFrontFaceOrientationType{false}
        , mForcedFrontFaceOrientationType{GLType::FrontFaceOrientation::CounterClockwise}
        , mShowOrientations{false}
        , mShowBoundingBoxes{false}
        , mFillBoundingBoxes{false}
        , mShowCollisionGeometry{false}
        , mCylinders{}
        , mSpheres{}
        , mDepthViewerShader{"depthView", pGLState}
        , mVisualiseNormalShader{"visualiseNormal", pGLState}
        , mCollisionGeometryShader{"collisionGeometry", pGLState}
        , mDebugPoints{}
        , mDebugPointsVAO{}
        , mDebugPointsVBO{}
    {}
    OpenGLRenderer::ViewInformation::ViewInformation()
        : mViewMatrix{glm::identity<glm::mat4>()}
        , mViewPosition{0.f}
        , mProjection{glm::identity<glm::mat4>()}
        , mZNearPlane{0.1f}
        , mZFarPlane{100.f}
        , mFOV{45.f}
    {}

    OpenGLRenderer::OpenGLRenderer(System::SceneSystem& pSceneSystem, System::MeshSystem& pMeshSystem, System::TextureSystem& pTextureSystem)
        : mGLState{}
        , mScreenFramebuffer{}
        , mSceneSystem{pSceneSystem}
        , mMeshSystem{pMeshSystem}
        , pointLightDrawCount{0}
        , spotLightDrawCount{0}
        , directionalLightDrawCount{0}
        , mPostProcessingOptions{}
        , mUniformColourShader{"uniformColour" , mGLState}
        , mTextureShader{"texture1" , mGLState}
        , mScreenTextureShader{"screenTexture", mGLState}
        , mSkyBoxShader{"skybox", mGLState}
        , mViewInformation{}
        , mDebugOptions{mGLState}
    {
        Platform::Core::mWindowResizeEvent.subscribe(this, &OpenGLRenderer::onWindowResize);

        const auto [width, height] = Platform::Core::getWindow().size();
        mScreenFramebuffer.attachColourBuffer(width, height);
        mScreenFramebuffer.attachDepthBuffer(width, height);
        mGLState.setViewport(width, height);

        LOG("Constructed new OpenGLRenderer instance");
    }

    void OpenGLRenderer::draw(const Data::Model& pModel)
    {
        draw(pModel.mCompositeMesh);
    }
    void OpenGLRenderer::draw(const Data::CompositeMesh& pComposite)
    {
        for (const auto& mesh : pComposite.mMeshes)
            draw(mesh);

        // Recursively draw all the child composites.
        for (const auto& childComposite : pComposite.mChildMeshes)
            draw(childComposite);
    }
    void OpenGLRenderer::draw(const Data::Mesh& pMesh)
    {
        const auto& GLMeshData = pMesh.mGLData;
        GLMeshData.mVAO.bind();

        if (GLMeshData.mEBO.has_value()) // EBO available means drawing with indices.
            mGLState.drawElements(GLType::PrimitiveMode::Triangles, GLMeshData.mDrawSize);
        else
            mGLState.drawArrays(GLType::PrimitiveMode::Triangles, GLMeshData.mDrawSize);
    }

    void OpenGLRenderer::draw()
    {
        { // Prepare mScreenFramebuffer for rendering
            mScreenFramebuffer.bind();
            mScreenFramebuffer.clearBuffers();
            ASSERT(mScreenFramebuffer.isComplete(), "Screen framebuffer not complete, have you attached a colour or depth buffer to it?");
        }

        { // Set global shader uniforms.
            if (auto* primaryCamera = mSceneSystem.getPrimaryCamera())
            {
                mViewInformation.mViewMatrix   = primaryCamera->getViewMatrix();
                mViewInformation.mViewPosition = primaryCamera->getPosition();
            }
            mViewInformation.mProjection = glm::perspective(glm::radians(mViewInformation.mFOV), Platform::Core::getWindow().aspectRatio(), mViewInformation.mZNearPlane, mViewInformation.mZFarPlane);
            mGLState.setUniformBlockVariable("ViewProperties.view", mViewInformation.mViewMatrix);
            mGLState.setUniformBlockVariable("ViewProperties.projection", mViewInformation.mProjection);
        }

        { // Setup the GL state for rendering the scene, the default setup can be overriden by the Debug options struct
            mGLState.setPolygonMode(GLType::PolygonMode::Fill);

            if (mDebugOptions.mForceClearColour)
                mGLState.setClearColour(mDebugOptions.mClearColour);
            else
                mGLState.setClearColour(glm::vec4(0.f));

            mGLState.toggleCullFaces(true);
            if (mDebugOptions.mForceCullFacesType)
                mGLState.setCullFacesType(mDebugOptions.mForcedCullFacesType);
            else
                mGLState.setCullFacesType(GLType::CullFacesType::Back);

            if (mDebugOptions.mForceFrontFaceOrientationType)
                mGLState.setFrontFaceOrientation(mDebugOptions.mForcedFrontFaceOrientationType);
            else
                mGLState.setFrontFaceOrientation(GLType::FrontFaceOrientation::CounterClockwise);

            mGLState.toggleDepthTest(true);
            if (mDebugOptions.mForceDepthTestType)
                mGLState.setDepthTestType(mDebugOptions.mForcedDepthTestType);
            else
                mGLState.setDepthTestType(GLType::DepthTestType::Less);

            if (mDebugOptions.mForceBlendType)
                mGLState.setBlendFunction(mDebugOptions.mForcedSourceBlendType, mDebugOptions.mForcedDestinationBlendType);
            else
                mGLState.setBlendFunction(GLType::BlendFactorType::SourceAlpha, GLType::BlendFactorType::OneMinusSourceAlpha);
        }

        auto scene = mSceneSystem.getCurrentScene();
        scene.foreach([&](ECS::Entity& pEntity, Component::Transform& pTransform, Component::Mesh& pMesh)
        {
            if (scene.hasComponents<Component::Texture>(pEntity))
            {
                auto& texComponent = scene.getComponent<Component::Texture>(pEntity);

                auto& shader = mTextureShader;
                shader.use(mGLState);
                shader.setUniform(mGLState, "model", pTransform.mModel);

                if (texComponent.mDiffuse.has_value())
                {
                    mGLState.setActiveTextureUnit(0);
                    texComponent.mDiffuse.value()->m_GL_texture.bind();
                }
                //   if (texComponent.mSpecular.has_value())
                //   {
                //       mGLState.setActiveTextureUnit(1);
                //       texComponent.mSpecular.value()->mGLTexture.bind();
                //   }
            }
            else
            {
                auto& shader = mUniformColourShader;
                shader.use(mGLState);
                shader.setUniform(mGLState, "model", pTransform.mModel);
                shader.setUniform(mGLState, "colour", glm::vec3(0.f, 1.f, 0.f));
            }

            draw(*pMesh.mModel);
        });

        renderDebug();
        //mGLState.renderImGui();

        { // Draw the colour output to the from mScreenFramebuffer texture to the default FBO
            // Unbind after completing draw to ensure all subsequent actions apply to the default FBO and not mScreenFrameBuffer.
            // Disable depth testing to not cull the screen quad the screen texture will be applied onto.
            FBO::unbind();
            mGLState.toggleDepthTest(false);
            mGLState.toggleCullFaces(false);
            mGLState.setPolygonMode(GLType::PolygonMode::Fill);
            glClear(GL_COLOR_BUFFER_BIT);

            mScreenTextureShader.use(mGLState);
            { // PostProcessing setters
                mScreenTextureShader.setUniform(mGLState, "invertColours", mPostProcessingOptions.mInvertColours);
                mScreenTextureShader.setUniform(mGLState, "grayScale", mPostProcessingOptions.mGrayScale);
                mScreenTextureShader.setUniform(mGLState, "sharpen", mPostProcessingOptions.mSharpen);
                mScreenTextureShader.setUniform(mGLState, "blur", mPostProcessingOptions.mBlur);
                mScreenTextureShader.setUniform(mGLState, "edgeDetection", mPostProcessingOptions.mEdgeDetection);
                mScreenTextureShader.setUniform(mGLState, "offset", mPostProcessingOptions.mKernelOffset);
            }

            mGLState.setActiveTextureUnit(0);
            mScreenFramebuffer.bindColourTexture();
            draw(*mMeshSystem.mPlanePrimitive);
        }

        //ASSERT(pointLightDrawCount == 4, "Only an exact number of 4 pointlights is supported.");
        //ASSERT(directionalLightDrawCount == 1, "Only one directional light is supported.");
        //ASSERT(spotLightDrawCount == 1, "Only one spotlight light is supported.");
        pointLightDrawCount       = 0;
        directionalLightDrawCount = 0;
        spotLightDrawCount        = 0;
    }

    void OpenGLRenderer::drawArrow(const glm::vec3& pOrigin, const glm::vec3& pDirection, const float pLength, const glm::vec3& pColour /*= glm::vec3(1.f,1.f,1.f)*/)
    {
        // Draw an arrow starting at pOrigin of length pLength point in pOrientation.
        // The body/stem of the arrow is a cylinder, the head/tip is a cone model.
        // We use seperate models for both to preserve the proportions which would be lost if we uniformly scaled an 'arrow mesh'

        static const float lengthToBodyLength  = 0.8f; //= 0.714f; // The proportion of the arrow that is the body.
        static const float lengthToBodyDiameter = 0.1f; // The factor from the length of the arrow to the diameter of the body.
        static const float bodyToHeadDiameter = 2.f; // The factor from the diamater of the body to the diameter of the head.

        // Model constants
        static const float cylinderDimensions = 2.f; // The default cylinder model has x, y and z dimensions in the range = [-1 - 1]
        static const float coneDimensions = 2.f;     // The default cone model has x, y and z dimensions in the range     = [-1 - 1]
        static const glm::vec3 modelDirection{0.f, 1.f, 0.f};                // Unit vec up, cone/cylinder models are alligned up (along y) by default.

        // Find the dimensions using pLength
        const float arrowBodyLength = pLength * lengthToBodyLength;
        const float arrowHeadLength = pLength - arrowBodyLength;
        const float arrowBodyDiameter = pLength * lengthToBodyDiameter;
        const float arrowHeadDiameter = arrowBodyDiameter * bodyToHeadDiameter;
        // The rotation to apply to make the arrow mesh point in pDirection.
        const auto arrowToDirectionRot = glm::mat4_cast(Utility::getRotation(modelDirection, pDirection));

        // CYLINDER/BODY
        const glm::vec3 arrowBodyCenter = pOrigin + (pDirection * (arrowBodyLength / 2.f)); // The center of the cylinder.
        const glm::vec3 arrowBodyScale = glm::vec3(arrowBodyDiameter / cylinderDimensions, arrowBodyLength / cylinderDimensions, arrowBodyDiameter / cylinderDimensions);
        glm::mat4 arrowBodyModel = glm::translate(glm::identity<glm::mat4>(), arrowBodyCenter);
        arrowBodyModel = arrowBodyModel * arrowToDirectionRot;
        arrowBodyModel = glm::scale(arrowBodyModel, arrowBodyScale);

        // CONE/HEAD
        const glm::vec3 arrowHeadPosition = pOrigin + (pDirection * (arrowBodyLength + (arrowHeadLength / 2.f))); // The center of the cone
        const glm::vec3 arrowHeadScale = glm::vec3(arrowHeadDiameter / coneDimensions, arrowHeadLength / coneDimensions, arrowHeadDiameter / coneDimensions);
        glm::mat4 arrowHeadModel = glm::translate(glm::identity<glm::mat4>(), arrowHeadPosition);
        arrowHeadModel = arrowHeadModel * arrowToDirectionRot;
        arrowHeadModel = glm::scale(arrowHeadModel, arrowHeadScale);

        mUniformColourShader.setUniform(mGLState, "colour", pColour);

        mUniformColourShader.setUniform(mGLState, "model", arrowHeadModel);
        draw(*mMeshSystem.mConePrimitive);

        mUniformColourShader.setUniform(mGLState, "model", arrowBodyModel);
        draw(*mMeshSystem.mCylinderPrimitive);
    }
    void OpenGLRenderer::drawCylinder(const glm::vec3& pStart, const glm::vec3& pEnd, const float pDiameter, const glm::vec3& pColour /*= glm::vec3(1.f,1.f,1.f)*/)
    {
        drawCylinder({pStart, pEnd, pDiameter}, pColour);
    }
    void OpenGLRenderer::drawCylinder(const Geometry::Cylinder& pCylinder, const glm::vec3& pColour /*=glm::vec3(1.f, 1.f, 1.f)*/)
    {
        static const float cylinderDimensions = 2.f; // The default cylinder model has x, y and z dimensions in the range = [-1 - 1]
        static const glm::vec3 cylinderAxis{0.f, 1.f, 0.f}; // Unit vec up, cone/cylinder models are alligned up (along y) by default.

        const float length = glm::length(pCylinder.mTop - pCylinder.mBase);
        const glm::vec3 direction = glm::normalize(pCylinder.mTop - pCylinder.mBase);
        const glm::vec3 center = pCylinder.mBase + (direction * (length / 2.f)); // The center of the cylinder in world space.
        const glm::mat4 rotation = glm::mat4_cast(glm::rotation(cylinderAxis, direction));
        // Cylinder model is alligned along the y-axis, scale the x and z to diameter and y to length before rotating.
        const glm::vec3 scale = glm::vec3(pCylinder.mDiameter / cylinderDimensions, length / cylinderDimensions, pCylinder.mDiameter / cylinderDimensions);

        glm::mat4 modelMat = glm::translate(glm::identity<glm::mat4>(), center);
        modelMat = modelMat * rotation;
        modelMat = glm::scale(modelMat, scale);

        mUniformColourShader.setUniform(mGLState, "colour", pColour);
        mUniformColourShader.setUniform(mGLState, "model", modelMat);
        draw(*mMeshSystem.mCylinderPrimitive);
    }
    void OpenGLRenderer::drawSphere(const glm::vec3& pCenter, const float& pRadius, const glm::vec3& pColour/*= glm::vec3(1.f, 1.f, 1.f)*/)
    {
        drawSphere({pCenter, pRadius}, pColour);
    }
    void OpenGLRenderer::drawSphere(const Geometry::Sphere& pSphere, const glm::vec3& pColour/*= glm::vec3(1.f, 1.f, 1.f)*/)
    {
        static const float sphereModelRadius = 1.f; // The default sphere model has XYZ dimensions in the range [-1 - 1] = radius of 1.f, diameter 2.f

        glm::mat4 modelMat = glm::translate(glm::identity<glm::mat4>(), pSphere.mCenter);
        modelMat = glm::scale(modelMat, glm::vec3(pSphere.mRadius / sphereModelRadius));

        mUniformColourShader.setUniform(mGLState, "colour", pColour);
        mUniformColourShader.setUniform(mGLState, "model", modelMat);
        draw(*mMeshSystem.mSpherePrimitive);
    }

    void OpenGLRenderer::renderDebug()
    {
        auto scene = mSceneSystem.getCurrentScene();
        mGLState.toggleCullFaces(true);

        {// Render all the collision geometry by pushing all the mesh triangles transformed in world-space.
            mDebugOptions.mDebugPoints.clear();
            mDebugOptions.mDebugPointsVAO.bind();
            mDebugOptions.mDebugPointsVBO.clear();

            if (mDebugOptions.mShowCollisionGeometry)
            {
                mSceneSystem.getCurrentScene().foreach([&](Component::Transform& pTransform, Component::Mesh& pMesh)
                {
                    mDebugOptions.mCollisionGeometryShader.use(mGLState);

                    pMesh.mModel->mCompositeMesh.forEachMesh([this, &pTransform](const Data::Mesh& pMesh)
                    {
                        for (auto triangle : pMesh.mTriangles)
                        {
                            triangle.transform(pTransform.mModel);
                            mDebugOptions.mDebugPoints.insert(mDebugOptions.mDebugPoints.end(), {triangle.mPoint1, triangle.mPoint2, triangle.mPoint3});
                        }
                    });
                });

                if (!mDebugOptions.mDebugPoints.empty())
                {
                    mDebugOptions.mDebugPointsVAO.bind();
                    mDebugOptions.mDebugPointsVBO = VBO();
                    mDebugOptions.mDebugPointsVBO.bind();
                    mDebugOptions.mDebugPointsVBO.setData(mDebugOptions.mDebugPoints, Shader::Attribute::Position3D);

                    mGLState.toggleCullFaces(false); // Disable culling to show exact geometry
                    mDebugOptions.mCollisionGeometryShader.setUniform(mGLState, "viewPosition", mViewInformation.mViewPosition);
                    mDebugOptions.mCollisionGeometryShader.setUniform(mGLState, "colour", glm::vec4(0.f, 1.f, 0.f, 0.5f));
                    mDebugOptions.mCollisionGeometryShader.setUniform(mGLState, "model", glm::mat4(1.f));
                    mGLState.drawArrays(GLType::PrimitiveMode::Triangles, static_cast<int>(mDebugOptions.mDebugPoints.size()));
                }
            }
        }

        if (mDebugOptions.mShowLightPositions)
        {
            mUniformColourShader.use(mGLState);

            mSceneSystem.getCurrentScene().foreach([this](Component::PointLight& pPointLight)
            {
                mUniformColourShader.setUniform(mGLState, "model", Utility::GetModelMatrix(pPointLight.mPosition, glm::vec3(0.f), glm::vec3(0.1f)));
                mUniformColourShader.setUniform(mGLState, "colour", pPointLight.mColour);
                draw(*mMeshSystem.mCubePrimitive);
            });
        }

        drawArrow(glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f), 1.f, glm::vec3(1.f, 0.f, 0.f));
        drawArrow(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 1.f, glm::vec3(0.f, 1.f, 0.f));
        drawArrow(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f), 1.f, glm::vec3(0.f, 0.f, 1.f));

        {// Draw debug shapes
            for (const auto& cylinder : mDebugOptions.mCylinders)
                drawCylinder(cylinder);
            for (const auto& sphere : mDebugOptions.mSpheres)
                drawSphere(sphere);
        }

        if (mDebugOptions.mShowBoundingBoxes)
        {
            mGLState.setPolygonMode(mDebugOptions.mFillBoundingBoxes ? GLType::PolygonMode::Fill : GLType::PolygonMode::Line);
            mUniformColourShader.use(mGLState);

            scene.foreach([&](Component::Transform& pTransform, Component::Mesh& pMesh, Component::Collider& pCollider)
            {
                mUniformColourShader.setUniform(mGLState, "model", pCollider.getWorldAABBModel());
                mUniformColourShader.setUniform(mGLState, "colour", pCollider.mCollided ? glm::vec3(1.f, 0.f, 0.f) : glm::vec3(0.f, 1.f, 0.f));
                draw(*mMeshSystem.mCubePrimitive);
            });
        }

        if (mDebugOptions.mShowOrientations)
        {
            scene.foreach([&](Component::Transform& pTransform, Component::Mesh& pMesh)
            {
                drawArrow(pTransform.mPosition, pTransform.mDirection, 1.f);
            });
        }
    }

    void OpenGLRenderer::setupLights()
    {
        mSceneSystem.getCurrentScene().foreach([this](Component::PointLight& pPointLight)
        {
            setShaderVariables(pPointLight);
        });
        mSceneSystem.getCurrentScene().foreach([this](Component::DirectionalLight& pDirectionalLight)
        {
            setShaderVariables(pDirectionalLight);
        });
        mSceneSystem.getCurrentScene().foreach([this](Component::SpotLight& pSpotLight)
        {
            setShaderVariables(pSpotLight);
        });
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

    void OpenGLRenderer::onWindowResize(const int pWidth, const int pHeight)
    {
        mScreenFramebuffer.resize(pWidth, pHeight);
        mGLState.setViewport(pWidth, pHeight);
    }

    glm::vec3 OpenGLRenderer::getCursorWorldDirection() const
    {
        const auto [mouseX, mouseY]   = Platform::Core::getCursorPosition(); // VIEWPORT
        ASSERT(mouseX > 0.f && mouseY > 0.f, "Mouse coordinates cannot be negative, did you miss a Window::capturingMouse() check before calling")
        const auto [windowX, windowY] = Platform::Core::getWindow().size();

        // VIEWPORT [0 - WINDOWSIZE] to OpenGL NDC [-1 - 1]
        const glm::vec2 normalizedDisplayCoords = glm::vec2((2.f * mouseX) / windowX - 1.f, (2.f * mouseY) / windowY - 1.f);

        // NDC to CLIPSPACE - Reversing normalizedDisplayCoords.y -> OpenGL windowSpace is relative to bottom left, getCursorPosition returns screen coordinates relative to top-left
        const glm::vec4 clipSpaceRay = glm::vec4(normalizedDisplayCoords.x, -normalizedDisplayCoords.y, -1.f, 1.f);

        // CLIPSPACE to EYE SPACE
        auto eyeSpaceRay = glm::inverse(mViewInformation.mProjection) * clipSpaceRay;
        eyeSpaceRay      = glm::vec4(eyeSpaceRay.x, eyeSpaceRay.y, -1.f, 0.f); // Set the direction into the screen -1.f

        // EYE SPACE to WORLD SPACE
        const glm::vec3 worldSpaceRay = glm::normalize(glm::vec3(glm::inverse(mViewInformation.mViewMatrix) * eyeSpaceRay));
        return worldSpaceRay;
    }
    Geometry::Ray OpenGLRenderer::getCursorWorldRay() const
    {
        return Geometry::Ray(mSceneSystem.getPrimaryCamera()->getPosition(), getCursorWorldDirection());
    }

} // namespace OpenGL