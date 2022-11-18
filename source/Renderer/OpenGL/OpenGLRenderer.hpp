#pragma once

// OPENGL
#include "GLState.hpp"
#include "OpenGLWindow.hpp"
#include "Shader.hpp"

// ECS
#include "Storage.hpp"

// DATA
#include "Mesh.hpp"
#include "Transform.hpp"

// UTILITY
#include "Utility.hpp"

// GLM
#include "glm/mat4x4.hpp"

// STD
#include <array>
#include <optional>
#include <unordered_map>
#include <vector>

struct GladGLContext;

namespace ECS
{
    class Storage;
}
namespace Component
{
    struct PointLight;
    struct DirectionalLight;
    struct SpotLight;
}
namespace System
{
    class MeshSystem;
    class TextureSystem;
}

namespace OpenGL
{
    // OpenGLRenderer uses Transform, MeshDraw, Pointlight, DirectionalLight and SpotLight components in the ECS to execute OpenGL draw commands.
    // Transform and MeshDraw components are cached into a more parsable format in mDrawcalls. In Listener functions DrawCalls are updated.
    // At construction, OpenGLRenderer parses all the Component::Texture and Component::Mesh files into an OpenGL::Texture and OpenGL::Mesh.
    class OpenGLRenderer
    {
    public:
        // OpenGLRenderer reads and renders the current state of pStorage when draw() is called.
        OpenGLRenderer(ECS::Storage& pStorage, const System::MeshSystem& pMeshSystem, const System::TextureSystem& pTextureSystem);
        ~OpenGLRenderer();

    private:

        glm::mat4 mViewMatrix;   // The view matrix used in draw(), set in setView
        glm::vec3 mViewPosition; // The view position used in draw(), set in setViewPosition
        glm::mat4 mProjection;

        const int cOpenGLVersionMajor, cOpenGLVersionMinor;
        // By default, OpenGL projection uses non-linear depth values (they have a very high precision for small z-values and a low precision for large z-values).
        // By setting this to true, BufferDrawType::Depth will visualise the values in a linear fashion from mZNearPlane to mZFarPlane.
        bool mLinearDepthView;
        bool mVisualiseNormals;
        bool mUseInstancedDraw;        // When possible this renderer will use DrawInstanced to more efficiently render lots of the same objects.
        float mZNearPlane;
        float mZFarPlane;
        float mFOV;

        // The window and GLAD context must be first declared to enforce the correct initialisation order:
        // ***************************************************************************************************
        OpenGLWindow mWindow;        // GLFW window which both GLFWInput and OpenGLRenderer depend on for their construction.
        GladGLContext* mGLADContext; // Depends on OpenGLWindow being initialised first. Must be declared after mWindow.
        GLState mGLState;

        size_t mTexture1ShaderIndex;
        size_t mTexture2ShaderIndex;
        size_t mUniformShaderIndex;
        size_t mMaterialShaderIndex;
        size_t mLightMapIndex;
        size_t mTexture1InstancedShaderIndex;
        Component::TextureID mMissingTextureID;
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

        GLData::FBO mMainScreenFBO;
        Component::MeshID mScreenQuad;
        Shader mScreenTextureShader;

        Component::MeshID mSkyBoxMeshID;
        Shader mSkyBoxShader;

        Shader mDepthViewerShader;
        Shader mVisualiseNormalShader;

        Component::MeshID m3DCubeID;
        Shader mLightEmitterShader;

        struct OpenGLMesh
        {
            Component::MeshID mID;

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
            std::vector<OpenGLMesh> mChildMeshes;
        };
        std::vector<OpenGLMesh> mGLMeshes;
        std::vector<GLData::Texture> mTextures; // Mapping of Component::Texture to OpenGL::Texture.
        std::vector<GLData::Texture> mCubeMaps; // Mapping of Component::CubeMapTexture to OpenGL::Texture.

        ECS::Storage& mStorage;

    public:
        void preDraw();
        void draw();
        void postDraw();
        void endFrame();

        void setupLights(const bool& pRenderLightPositions);

        void newImGuiFrame();
        void renderImGuiFrame();
        void renderImGui();

        void initialiseMesh(const Component::Mesh& pMesh);
        void initialiseTexture(const Component::Texture& pTexture);
        void initialiseCubeMap(const Component::CubeMapTexture& pCubeMap);
        void setView(const glm::mat4& pViewMatrix) { mViewMatrix = pViewMatrix; }
        void setViewPosition(const glm::vec3& pViewPosition) { mViewPosition = pViewPosition; }

    private:
        Shader* getShader(Component::MeshDraw& pMeshDraw);
        void setShaderVariables(const Component::PointLight& pPointLight);
        void setShaderVariables(const Component::DirectionalLight& pDirectionalLight);
        void setShaderVariables(const Component::SpotLight& pSpotLight);

        // Holds all the constructed instances of OpenGLRenderer to allow calling non-static member functions.
        inline static std::vector<OpenGLRenderer*> OpenGLInstances;
        void onResize(const int pWidth, const int pHeight);

        // Get all the data required to draw this mesh in its default configuration.
        const OpenGLMesh& getGLMesh(const Component::MeshID& pMeshID) const;
        const GLData::Texture& getTexture(const Component::TextureID& pTextureID) const;

        // Recursively draw the OpenGLMesh and all its children.
        void draw(const OpenGLMesh& pMesh, const size_t& pInstancedCount = 0);

        static GladGLContext* initialiseGLAD();                                       // Requires a GLFW window to be set as current context, done in OpenGLWindow constructor
        static void windowSizeCallback(GLFWwindow* pWindow, int pWidth, int pHeight); // Callback required by GLFW to be static/global.
    };
} // namespace OpenGL