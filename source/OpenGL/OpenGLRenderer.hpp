#pragma once

// OPENGL
#include "GLState.hpp"
#include "Types.hpp"
#include "Shader.hpp"
#include "PhongRenderer.hpp"
#include "LightPositionRenderer.hpp"
#include "ShadowMapper.hpp"
#include "OpenGL/ParticleRenderer.hpp"

// GEOMETRY
#include "Cylinder.hpp"
#include "Sphere.hpp"

// GLM
#include "glm/fwd.hpp"
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"

#include "Component/Texture.hpp"
#include "Component/Mesh.hpp"

#include "Utility/ResourceManager.hpp"

// STD
#include <filesystem>
#include <memory>
#include <optional>
#include <vector>

namespace ECS
{
    class Storage;
}
namespace Component
{
    class Mesh;
    class Texture;
    struct PointLight;
    struct DirectionalLight;
    struct SpotLight;
}
namespace System
{
    class MeshSystem;
    class TextureSystem;
    class SceneSystem;
}
namespace Data
{
    class Texture;
    class Model;
}
namespace Geometry
{
    class Cylinder;
    class Sphere;
}
namespace Platform
{
    class Window;
}

namespace OpenGL
{
    // OpenGLRenderer renders the state of the ECS::Storage when draw is called.
    class OpenGLRenderer
    {
		static Data::NewMesh get_origin_arrows_mesh();

        struct DebugOptions
        {
            DebugOptions();

            // Rendering
            bool mShowLightPositions;
            bool mVisualiseNormals;

            bool mForceClearColour;
            glm::vec4 mClearColour;

            bool mForceDepthTestType;
            DepthTestType mForcedDepthTestType;

            bool mForceBlendType;
            BlendFactorType mForcedSourceBlendType;
            BlendFactorType mForcedDestinationBlendType;

            bool mForceCullFaceType;
            CullFaceType mForcedCullFaceType;

            bool mForceFrontFaceOrientationType;
            FrontFaceOrientation mForcedFrontFaceOrientationType;

            // Physics
            bool mShowOrientations;
            bool mShowBoundingBoxes;
            bool mFillBoundingBoxes;
            bool mShowCollisionGeometry; // Display all Meshes according to their collision geometry

            std::vector<Geometry::Cylinder> mCylinders;
            std::vector<Geometry::Sphere> mSpheres;

            Shader mDepthViewerShader;
            Shader mVisualiseNormalShader;

            Shader mCollisionGeometryShader;
            std::vector<glm::vec3> mDebugPoints;
            VAO mDebugPointsVAO;
            VBO mDebugPointsVBO;
        };
        struct PostProcessingOptions
        {
            bool mInvertColours = false;
            bool mGrayScale     = false;
            bool mSharpen       = false;
            bool mBlur          = false;
            bool mEdgeDetection = false;
            float mKernelOffset = 1.0f / 300.0f;
        };
        struct ViewInformation
        {
            ViewInformation();

            glm::mat4 mView;
            glm::vec3 mViewPosition;
            glm::mat4 mProjection;
        };

        Platform::Window& m_window;
        FBO mScreenFramebuffer;

        System::SceneSystem& mSceneSystem;
        System::MeshSystem& mMeshSystem;

        Shader mUniformColourShader;
        Shader m_colour_shader;
        Shader mTextureShader;
        Shader mScreenTextureShader;
        Shader mSkyBoxShader;

        PhongRenderer m_phong_renderer;
        ParticleRenderer m_particle_renderer;
        LightPositionRenderer m_light_position_renderer;
        ShadowMapper m_shadow_mapper;
        TextureRef m_missing_texture;
        TextureRef m_blank_texture;
        ModelRef m_cube;
		Data::NewMesh m_origin_arrows;

    public:
        ViewInformation mViewInformation;
        PostProcessingOptions mPostProcessingOptions;
        DebugOptions mDebugOptions;

        // OpenGLRenderer reads and renders the current state of pStorage when draw() is called.
        OpenGLRenderer(Platform::Window& p_window, System::SceneSystem& pSceneSystem, System::MeshSystem& pMeshSystem, System::TextureSystem& pTextureSystem) noexcept;

        void start_frame();
        void end_frame();
        // Draw the current state of the ECS.
        void draw(const DeltaTime& delta_time);

    private:

        void draw(const Data::Model& pModel);
        void draw(const Data::CompositeMesh& pComposite);
        void draw(const Data::Mesh& pMesh);

        void renderDebug();
        void drawArrow(const glm::vec3& pOrigin, const glm::vec3& pDirection, const float pLength, const glm::vec3& p_colour = glm::vec3(1.f, 1.f, 1.f)); // Draw an arrow from pOrigin in pDirection of pLength.
        void drawCylinder(const glm::vec3& pStart, const glm::vec3& pEnd, const float pDiameter, const glm::vec3& p_colour = glm::vec3(1.f, 1.f, 1.f)); // Draw a cylinder with base at pStart and top at pEnd of pDiameter.
        void drawCylinder(const Geometry::Cylinder& pCylinder, const glm::vec3& p_colour = glm::vec3(1.f, 1.f, 1.f));
        void drawSphere(const glm::vec3& pCenter, const float& pRadius, const glm::vec3& p_colour = glm::vec3(1.f, 1.f, 1.f));
        void drawSphere(const Geometry::Sphere& pSphere, const glm::vec3& p_colour = glm::vec3(1.f, 1.f, 1.f));
    };
} // namespace OpenGL