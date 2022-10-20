#pragma once

#include "OpenGLWindow.hpp"
#include "GLState.hpp"
#include "Shader.hpp"

#include "GraphicsAPI.hpp"

#include "Utility.hpp"
#include "Types.hpp"

// STD
#include <unordered_map>
#include <optional>
#include <array>
#include <vector>
#include <string>

struct GladGLContext;
struct DrawCall;
struct Mesh;

namespace ECS
{
	class EntityManager;
}
namespace Data
{
	struct PointLight;
	struct DirectionalLight;
	struct SpotLight;
}

class OpenGLAPI : public GraphicsAPI
{
public:
	OpenGLAPI(const ECS::EntityManager& pEntityManager);
	~OpenGLAPI();

	void preDraw() 										 		override;
	void draw()							 				 		override;
	void draw(const Data::PointLight& pPointLight) 			    override;
	void draw(const Data::DirectionalLight& pDirectionalLight)  override;
	void draw(const Data::SpotLight& pSpotLight) 				override;
	void postDraw() 									 		override;
	void endFrame() 									 		override;

	void newImGuiFrame()										override;
	void renderImGuiFrame() 									override;
	void renderImGui()											override;

	void initialiseMesh(const Data::Mesh& pMesh) 			 	override;
	void initialiseTexture(const Texture& pTexture)  	   		override;
	void initialiseCubeMap(const CubeMapTexture& pCubeMap) 		override;


	// Listeners
	void onEntityCreated(const ECS::Entity& pEntity, const ECS::EntityManager& pManager) 			override;
	void onTransformComponentAdded(const ECS::Entity& pEntity, const Data::Transform& pTransform) 	override;
	void onTransformComponentChanged(const ECS::Entity& pEntity, const Data::Transform& pTransform) override;
	void onMeshComponentAdded(const ECS::Entity& pEntity, const Data::MeshDraw& pMesh) 				override;

private:
	// Using the mesh and tranform component assigned to an Entity, construct a DrawCall for it.
	void addEntityDrawCall(const ECS::Entity& pEntity, const Data::Transform& pTransform, const Data::MeshDraw& pMesh);

	// Holds all the constructed instances of OpenGLAPI to allow calling non-static member functions.
	inline static std::vector<OpenGLAPI*> OpenGLInstances;
	void onResize(const int pWidth, const int pHeight);

	struct OpenGLMesh
	{
		MeshID mID;

		GLType::PrimitiveMode mDrawMode = GLType::PrimitiveMode::Triangles;
		int mDrawSize = -1; // Cached size of data used in OpenGL draw call, either size of Mesh positions or indices
		enum class DrawMethod { Indices, Array, Null };
		DrawMethod mDrawMethod = DrawMethod::Null;

		GLData::VAO mVAO;
		std::optional<GLData::EBO> mEBO;
		std::array<std::optional<GLData::VBO>, util::toIndex(Shader::Attribute::Count)> mVBOs;

		// Composite mesh
		std::vector<OpenGLMesh> mChildMeshes;
	};

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

	static GladGLContext* initialiseGLAD(); // Requires a GLFW window to be set as current context, done in OpenGLWindow constructor
	static void windowSizeCallback(GLFWwindow* pWindow, int pWidth, int pHeight); // Callback required by GLFW to be static/global.

	const int cOpenGLVersionMajor, cOpenGLVersionMinor;
	// By default, OpenGL projection uses non-linear depth values (they have a very high precision for small z-values and a low precision for large z-values).
	// By setting this to true, BufferDrawType::Depth will visualise the values in a linear fashion from mZNearPlane to mZFarPlane.
	bool mLinearDepthView;
	bool mVisualiseNormals;
	bool mUseInstancedDraw; // When possible this renderer will use DrawInstanced to more efficiently render lots of the same objects.
	int mInstancingCountThreshold; // When a DrawCall is repeated this many times, it is marked as a candidate for instanced rendering. To qualify, the DrawCall must also have an instanced compatible shader retrieved by getShader.
	float mZNearPlane;
	float mZFarPlane;
	float mFOV;

	// The window and GLAD context must be first declared to enforce the correct initialisation order:
	// ***************************************************************************************************
	OpenGLWindow mWindow;		 // GLFW window which both GLFWInput and OpenGLAPI depend on for their construction.
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
		bool mGrayScale 	= false;
		bool mSharpen 		= false;
		bool mBlur 			= false;
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

	std::vector<Shader> mAvailableShaders; // Has one of every type of shader usable by DrawCalls. Found in the GLSL folder.
	std::vector<std::optional<Shader>> mDrawCallToShader; // 1-1 mapping of mDrawCalls to Shader they are using to render.

	std::vector<OpenGLMesh> mGLMeshes;
	std::vector<GLData::Texture> mTextures; // Mapping of Data::Texture to OpenGL::Texture.
	std::vector<GLData::Texture> mCubeMaps; // Mapping of Data::CubeMapTexture to OpenGL::Texture.
};