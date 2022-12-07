#pragma once

// OPENGL
#include "GLState.hpp"
#include "Shader.hpp"

// GEOMETRY
#include "Cylinder.hpp"
#include "Sphere.hpp"

// UTILITY
#include "Utility.hpp"

// GLM
#include "glm/fwd.hpp"
#include "glm/mat4x4.hpp"

// STD
#include <array>
#include <optional>
#include <vector>

namespace ECS
{
    class Storage;
}
namespace Component
{
    struct Mesh;
    struct MeshDraw;
    struct TextureID;
    struct Texture;
    struct CubeMapTexture;
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
    // The components is concerns itself with are Transform, MeshDraw, PointLight, SpotLight, DirectionalLight
    // At construction, OpenGLRenderer parses all the Component::Texture and Component::Mesh files into an OpenGL::Texture and OpenGL::GLMeshData.
    class OpenGLRenderer
    {
    public:
        // OpenGLRenderer reads and renders the current state of pStorage when draw() is called.
        OpenGLRenderer(System::SceneSystem& pSceneSystem, const System::MeshSystem& pMeshSystem, const System::TextureSystem& pTextureSystem);

        void draw();

        void setupLights();
        void renderImGui();

        // Returns the current cursor screen position as a normalised direction vector in world-space.
        // This function is OpenGL specific as it makes assumptions on NDC coordinate system and forward direction in world space.
        glm::vec3 getCursorWorldDirection() const;

        // List of cylinders to draw for debugging purposes.
        std::vector<Geometry::Cylinder> debugCylinders;
        std::vector<Geometry::Sphere> debugSpheres;
    private:
        GLState mGLState;
        GLData::FBO mMainScreenFBO;

        System::SceneSystem& mSceneSystem;
        const System::MeshSystem& mMeshSystem;

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

        std::vector<Shader> mAvailableShaders; // Has one of every type of shader usable by DrawCalls. Found in the GLSL folder.
        size_t mTexture1ShaderIndex;
        size_t mTexture2ShaderIndex;
        size_t mMaterialShaderIndex;
        size_t mUniformShaderIndex;
        size_t mLightMapIndex;
        size_t mTexture1InstancedShaderIndex;
        Shader mScreenTextureShader;
        Shader mSkyBoxShader;
        Shader mLightEmitterShader;
        Shader mDepthViewerShader;
        Shader mVisualiseNormalShader;

        struct GLMeshData
        {
            GLType::PrimitiveMode mDrawMode = GLType::PrimitiveMode::Triangles;
            int mDrawSize                   = -1; // Cached size of data used in OpenGL draw call, either size of Mesh positions or indices
            enum class DrawMethod
            {
                Indices,
                Array,
                Null
            };
            DrawMethod mDrawMethod = DrawMethod::Null;

            GLData::VAO mVAO;
            std::optional<GLData::EBO> mEBO;
            std::array<std::optional<GLData::VBO>, Utility::toIndex(Shader::Attribute::Count)> mVBOs;

            // Composite mesh
            std::vector<GLMeshData> mChildMeshes;
        };
        std::vector<GLMeshData> mGLMeshData;
        size_t m3DCubeMeshIndex;
        size_t mSkyBoxMeshIndex;
        size_t mScreenQuadMeshIndex;
        size_t mCylinderIndex;
        size_t mConeIndex;
        size_t mSphereIndex;

        std::vector<GLData::Texture> mTextures; // Mapping of Component::Texture to OpenGL::Texture.
        size_t mMissingTextureID;
        std::vector<GLData::Texture> mCubeMaps; // Mapping of Component::CubeMapTexture to OpenGL::Texture.

        Shader* getShader(Component::MeshDraw& pMeshDraw);

        void setShaderVariables(const Component::PointLight& pPointLight);
        void setShaderVariables(const Component::DirectionalLight& pDirectionalLight);
        void setShaderVariables(const Component::SpotLight& pSpotLight);
        void draw(const GLMeshData& pMesh, const size_t& pInstancedCount = 0); // Recursively draw the GLMeshData and all its children.
        void drawArrow(const glm::vec3& pOrigin, const glm::vec3& pDirection, const float pLength, const glm::vec3& pColour = glm::vec3(1.f, 1.f, 1.f)); // Draw an arrow from pOrigin in pDirection of pLength.
        void drawCylinder(const glm::vec3& pStart, const glm::vec3& pEnd, const float pDiameter, const glm::vec3& pColour = glm::vec3(1.f, 1.f, 1.f)); // Draw a cylinder with base at pStart and top at pEnd of pDiameter.
        void drawCylinder(const Geometry::Cylinder& pCylinder, const glm::vec3& pColour = glm::vec3(1.f, 1.f, 1.f));
        void drawSphere(const glm::vec3& pCenter, const float& pRadius, const glm::vec3& pColour = glm::vec3(1.f, 1.f, 1.f));
        void drawSphere(const Geometry::Sphere& pSphere, const glm::vec3& pColour = glm::vec3(1.f, 1.f, 1.f));

        void initialiseMesh(const Component::Mesh& pMesh, GLMeshData* pParentMesh = nullptr);
        void initialiseTexture(const Component::Texture& pTexture);
        void initialiseCubeMap(const Component::CubeMapTexture& pCubeMap);

        void onWindowResize(const int pWidth, const int pHeight);
    };
} // namespace OpenGL