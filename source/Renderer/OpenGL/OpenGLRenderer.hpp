#pragma once

// OPENGL
#include "GLState.hpp"
#include "OpenGLWindow.hpp"
#include "Shader.hpp"

// ECS
#include "Entity.hpp"

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
    class EntityManager;
}
namespace Data
{
    struct PointLight;
    struct DirectionalLight;
    struct SpotLight;
} // namespace Data

namespace OpenGL
{
    // OpenGLRenderer uses Transform, MeshDraw, Pointlight, DirectionalLight and SpotLight components in the ECS to execute OpenGL draw commands.
    // Transform and MeshDraw components are cached into a more parsable format in mDrawcalls. In Listener functions DrawCalls are updated.
    class OpenGLRenderer
    {
    private:
        // A request to execute a specific MeshDraw at a number of locations using a GraphicsAPI.
        // DrawCall's purpose is to group together the same MeshDraw's differentiating them only by the Model matrices found inside mModels.
        // For the above reason DrawCalls are a Zephyr::Renderer construct as they are fed to GraphicsAPI's in this more parsable format for instancing.
        struct DrawCall
        {
            Data::MeshDraw mMesh;

            // List of per-Entity transform matrices
            std::vector<glm::mat4> mModels;
            // Mapping of EntityID to index into mModels.
            std::unordered_map<ECS::EntityID, size_t> mEntityModelIndexLookup;
        };
        std::vector<DrawCall> mDrawCalls; // GraphicsAPI Executes all these DrawCalls using the draw function.

        glm::mat4 mViewMatrix;   // The view matrix used in draw(), set in setView
        glm::vec3 mViewPosition; // The view position used in draw(), set in setViewPosition
        glm::mat4 mProjection;

        const int cOpenGLVersionMajor, cOpenGLVersionMinor;
        // By default, OpenGL projection uses non-linear depth values (they have a very high precision for small z-values and a low precision for large z-values).
        // By setting this to true, BufferDrawType::Depth will visualise the values in a linear fashion from mZNearPlane to mZFarPlane.
        bool mLinearDepthView;
        bool mVisualiseNormals;
        bool mUseInstancedDraw;        // When possible this renderer will use DrawInstanced to more efficiently render lots of the same objects.
        int mInstancingCountThreshold; // When a DrawCall is repeated this many times, it is marked as a candidate for instanced rendering. To qualify, the DrawCall must also have an instanced compatible shader retrieved by getShader.
        float mZNearPlane;
        float mZFarPlane;
        float mFOV;

        // The window and GLAD context must be first declared to enforce the correct initialisation order:
        // ***************************************************************************************************
        OpenGLWindow mWindow;        // GLFW window which both GLFWInput and OpenGLRenderer depend on for their construction.
        GladGLContext* mGLADContext; // Depends on OpenGLWindow being initialised first. Must be declared after mWindow.
        GLState mGLState;

        const ECS::EntityManager& mEntityManager;

        size_t mTexture1ShaderIndex;
        size_t mTexture2ShaderIndex;
        size_t mUniformShaderIndex;
        size_t mMaterialShaderIndex;
        size_t mLightMapIndex;
        size_t mTexture1InstancedShaderIndex;
        TextureID mMissingTextureID;
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

        GLData::FBO mMainScreenFBO;
        MeshID mScreenQuad;
        Shader mScreenTextureShader;

        MeshID mSkyBoxMeshID;
        Shader mSkyBoxShader;

        Shader mDepthViewerShader;
        Shader mVisualiseNormalShader;

        MeshID m3DCubeID;
        Shader mLightEmitterShader;

        std::vector<Shader> mAvailableShaders;                // Has one of every type of shader usable by DrawCalls. Found in the GLSL folder.
        std::vector<std::optional<Shader>> mDrawCallToShader; // 1-1 mapping of mDrawCalls to Shader they are using to render.

        struct OpenGLMesh
        {
            MeshID mID;

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
            std::array<std::optional<GLData::VBO>, util::toIndex(Shader::Attribute::Count)> mVBOs;

            // Composite mesh
            std::vector<OpenGLMesh> mChildMeshes;
        };
        std::vector<OpenGLMesh> mGLMeshes;
        std::vector<GLData::Texture> mTextures; // Mapping of Data::Texture to OpenGL::Texture.
        std::vector<GLData::Texture> mCubeMaps; // Mapping of Data::CubeMapTexture to OpenGL::Texture.

    public:
        OpenGLRenderer(const ECS::EntityManager& pEntityManager);
        ~OpenGLRenderer();

        void preDraw();
        void draw();
        void postDraw();
        void endFrame();

        void setupLights(const bool& pRenderLightPositions);

        void newImGuiFrame();
        void renderImGuiFrame();
        void renderImGui();

        void initialiseMesh(const Data::Mesh& pMesh);
        void initialiseTexture(const Texture& pTexture);
        void initialiseCubeMap(const CubeMapTexture& pCubeMap);
        void setView(const glm::mat4& pViewMatrix) { mViewMatrix = pViewMatrix; }
        void setViewPosition(const glm::vec3& pViewPosition) { mViewPosition = pViewPosition; }

        // Listeners
        void onEntityCreated(const ECS::Entity& pEntity, const ECS::EntityManager& pManager);
        void onEntityRemoved(const ECS::Entity& pEntity, const ECS::EntityManager& pManager);

        void onTransformComponentAdded(const ECS::Entity& pEntity, const Data::Transform& pTransform);
        void onTransformComponentChanged(const ECS::Entity& pEntity, const Data::Transform& pTransform);
        void onTransformComponentRemoved(const ECS::Entity& pEntity);

        void onMeshComponentAdded(const ECS::Entity& pEntity, const Data::MeshDraw& pMesh);
        void onMeshComponentRemoved(const ECS::Entity& pEntity);

    private:
        // Using the mesh and tranform component assigned to an Entity, construct a DrawCall for it.
        void addEntityDrawCall(const ECS::Entity& pEntity, const Data::Transform& pTransform, const Data::MeshDraw& pMesh);
        void removeEntityDrawCall(const ECS::Entity& pEntity);

        void setShaderVariables(const Data::PointLight& pPointLight);
        void setShaderVariables(const Data::DirectionalLight& pDirectionalLight);
        void setShaderVariables(const Data::SpotLight& pSpotLight);

        // Holds all the constructed instances of OpenGLRenderer to allow calling non-static member functions.
        inline static std::vector<OpenGLRenderer*> OpenGLInstances;
        void onResize(const int pWidth, const int pHeight);

        // Get all the data required to draw this mesh in its default configuration.
        const OpenGLMesh& getGLMesh(const MeshID& pMeshID) const;
        const GLData::Texture& getTexture(const TextureID& pTextureID) const;
        // Returns the shader assigned to the DrawCall.
        // This can be overridden in the Draw function if mBufferDrawType is set to something other than BufferDrawType::Colour.
        Shader* getShader(const DrawCall& pDrawCall, const size_t& pDrawCallIndex);
        // Returns the instanced version of pShader.
        Shader* getInstancedShader(const Shader& pShader);

        // Checks if the current Shader assigned to pDrawCall is correct, assigns a new shader is it's not.
        // updateShader is called whenever data changes that might require a Shader change e.g.
        // Transform components added/removed/changed.
        // Instanced ImGUI flags change.
        bool updateShader(const DrawCall& pDrawCall, const size_t& pDrawCallIndex);
        // Recursively draw the OpenGLMesh and all its children.
        void draw(const OpenGLMesh& pMesh, const size_t& pInstancedCount = 0);
        // Called whenever mUseInstancedDraw changes.
        void onInstancedOptionChanged();

        static GladGLContext* initialiseGLAD();                                       // Requires a GLFW window to be set as current context, done in OpenGLWindow constructor
        static void windowSizeCallback(GLFWwindow* pWindow, int pWidth, int pHeight); // Callback required by GLFW to be static/global.
    };
} // namespace OpenGL