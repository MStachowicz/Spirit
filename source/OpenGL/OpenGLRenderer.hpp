#pragma once

// OPENGL
#include "GLState.hpp"
#include "Types.hpp"
#include "Shader.hpp"

// GEOMETRY
#include "Cylinder.hpp"
#include "Ray.hpp"
#include "Sphere.hpp"
#include "Triangle.hpp"

// GLM
#include "glm/fwd.hpp"
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"

// STD
#include <filesystem>
#include <memory>
#include <optional>
#include <vector>

namespace ECS
{
    class Storage;
}
namespace Data
{
    class Texture;
    class Model;
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

namespace OpenGL
{
    // OpenGLRenderer renders the state of the ECS::Storage when draw is called.
    class OpenGLRenderer
    {
        struct DebugOptions
        {
            DebugOptions(GLState& pGLState);

            bool mRendering; // Toggle this flag to display rendering visualisation aids.
                bool mShowLightPositions;
                bool mVisualiseNormals;
                glm::vec4 mClearColour;

                bool mForceDepthTestType;
                GLType::DepthTestType mForcedDepthTestType;

                bool mForceBlendType;
                GLType::BlendFactorType mForcedSourceBlendType;
                GLType::BlendFactorType mForcedDestinationBlendType;

                bool mForceCullFacesType;
                GLType::CullFacesType mForcedCullFacesType;

                bool mForceFrontFaceOrientationType;
                GLType::FrontFaceOrientation mForcedFrontFaceOrientationType;

            bool mPhysics; // Toggle this flag to display physics and collision visualisation aids.
                bool mShowOrientations;
                bool mShowBoundingBoxes;
                    bool mFillBoundingBoxes;
                bool mShowCollisionTriangles;

            std::vector<Geometry::Cylinder> mCylinders;
            std::vector<Geometry::Sphere> mSpheres;
            // Debug triangles are drawn directly from a buffer since they are GL primtives. Thus they are added and removed by addDebugTriangle and clearDebugTriangles.
            std::vector<glm::vec3> mTriangles;
            VAO mTriangleVAO;
            VBO mTriangleVBO;

            // By default, OpenGL projection uses non-linear depth values (they have a very high precision for small z-values and a low precision for large z-values).
            // By setting this to true, BufferDrawType::Depth will visualise the values in a linear fashion from mZNearPlane to mZFarPlane.
            bool mLinearDepthView;
            Shader mDepthViewerShader;
            Shader mVisualiseNormalShader;
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

            glm::mat4 mViewMatrix;
            glm::vec3 mViewPosition;
            glm::mat4 mProjection;
            float mZNearPlane;
            float mZFarPlane;
            float mFOV;
        };

        GLState mGLState;
        FBO mScreenFramebuffer;

        System::SceneSystem& mSceneSystem;
        System::MeshSystem& mMeshSystem;

        Shader mUniformColourShader;
        Shader mTextureShader;
        Shader mScreenTextureShader;
        Shader mSkyBoxShader;

        int pointLightDrawCount;
        int spotLightDrawCount;
        int directionalLightDrawCount;
    public:
        ViewInformation mViewInformation;
        PostProcessingOptions mPostProcessingOptions;
        DebugOptions mDebugOptions;
        void addDebugTriangle(const Geometry::Triangle& pTriangle);
        void clearDebugTriangles();
        void showGraphicsOptions();
        void showPhysicsOptions();


        // OpenGLRenderer reads and renders the current state of pStorage when draw() is called.
        OpenGLRenderer(System::SceneSystem& pSceneSystem, System::MeshSystem& pMeshSystem, System::TextureSystem& pTextureSystem);
        // Draw the current state of the ECS.
        void draw();

        // Returns the current cursor screen position as a normalised direction vector in world-space.
        // This function is OpenGL specific as it makes assumptions on NDC coordinate system and forward direction in world space.
        glm::vec3 getCursorWorldDirection() const;
        // Returns a ray cast from the primary camera view position in the direction of the cursor.
        Geometry::Ray getCursorWorldRay() const;

    private:
        void setShaderVariables(const Component::PointLight& pPointLight);
        void setShaderVariables(const Component::DirectionalLight& pDirectionalLight);
        void setShaderVariables(const Component::SpotLight& pSpotLight);
        void setupLights();

        void draw(const Data::Model& pModel);
        void draw(const Data::CompositeMesh& pComposite);
        void draw(const Data::Mesh& pMesh);

        void renderDebug();
        void drawArrow(const glm::vec3& pOrigin, const glm::vec3& pDirection, const float pLength, const glm::vec3& pColour = glm::vec3(1.f, 1.f, 1.f)); // Draw an arrow from pOrigin in pDirection of pLength.
        void drawCylinder(const glm::vec3& pStart, const glm::vec3& pEnd, const float pDiameter, const glm::vec3& pColour = glm::vec3(1.f, 1.f, 1.f)); // Draw a cylinder with base at pStart and top at pEnd of pDiameter.
        void drawCylinder(const Geometry::Cylinder& pCylinder, const glm::vec3& pColour = glm::vec3(1.f, 1.f, 1.f));
        void drawSphere(const glm::vec3& pCenter, const float& pRadius, const glm::vec3& pColour = glm::vec3(1.f, 1.f, 1.f));
        void drawSphere(const Geometry::Sphere& pSphere, const glm::vec3& pColour = glm::vec3(1.f, 1.f, 1.f));

        void onWindowResize(const int pWidth, const int pHeight);
    };
} // namespace OpenGL