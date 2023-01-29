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
    public:
        // OpenGLRenderer reads and renders the current state of pStorage when draw() is called.
        OpenGLRenderer(System::SceneSystem& pSceneSystem, System::MeshSystem& pMeshSystem, System::TextureSystem& pTextureSystem);

        // Draw the current state of the ECS.
        void draw();
        void renderImGui();

        // Returns the current cursor screen position as a normalised direction vector in world-space.
        // This function is OpenGL specific as it makes assumptions on NDC coordinate system and forward direction in world space.
        glm::vec3 getCursorWorldDirection() const;
        // Returns a ray cast from the primary camera view position in the direction of the cursor.
        Geometry::Ray getCursorWorldRay() const;

        // List of cylinders to draw for debugging purposes.
        std::vector<Geometry::Cylinder> debugCylinders;
        std::vector<Geometry::Sphere> debugSpheres;
        void addDebugTriangle(const Geometry::Triangle& pTriangle);
        void clearDebugTriangles();
    private:
        GLState mGLState;
        FBO mScreenFramebuffer;

        // Debug triangles are drawn directly from a buffer since they are GL primtives. Thus they are added and removed by addDebugTriangle and clearDebugTriangles.
        std::vector<glm::vec3> mDebugTriangles;
        VAO mTriangleVAO;
        VBO mTriangleBuffer;

        System::SceneSystem& mSceneSystem;
        System::MeshSystem& mMeshSystem;

        glm::mat4 mViewMatrix;
        glm::vec3 mViewPosition;
        glm::mat4 mProjection;

        // By default, OpenGL projection uses non-linear depth values (they have a very high precision for small z-values and a low precision for large z-values).
        // By setting this to true, BufferDrawType::Depth will visualise the values in a linear fashion from mZNearPlane to mZFarPlane.
        bool mLinearDepthView;
        bool mVisualiseNormals;
        bool mShowOrientations;
        bool mShowLightPositions;
        bool mShowBoundingBoxes;
        bool mFillBoundingBoxes;
        float mZNearPlane;
        float mZFarPlane;
        float mFOV;

        int pointLightDrawCount;
        int spotLightDrawCount;
        int directionalLightDrawCount;

        enum BufferDrawType
        {
            Colour,
            Depth,
            Count
        };
        BufferDrawType mBufferDrawType;

        struct PostProcessingOptions
        {
            bool mInvertColours = false;
            bool mGrayScale     = false;
            bool mSharpen       = false;
            bool mBlur          = false;
            bool mEdgeDetection = false;
            float mKernelOffset = 1.0f / 300.0f;
        };
        PostProcessingOptions mPostProcessingOptions;

        Shader mUniformColourShader;
        Shader mTextureShader;
        Shader mScreenTextureShader;
        Shader mSkyBoxShader;
        Shader mDepthViewerShader;
        Shader mVisualiseNormalShader;

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