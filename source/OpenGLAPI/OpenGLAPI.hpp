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

namespace Data
{
	struct PointLight;
	struct DirectionalLight;
	struct SpotLight;
}

class OpenGLAPI : public GraphicsAPI
{
public:
	OpenGLAPI();
	~OpenGLAPI();

	void preDraw() 										 		override;
	void draw(const DrawCall& pDrawCall) 				 		override;
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

private:
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
		GLData::EBO mEBO;
		std::array<std::optional<GLData::VBO>, util::toIndex(Shader::Attribute::Count)> mVBOs;

		// Composite mesh
		std::vector<OpenGLMesh> mChildMeshes;
	};

	// Get all the data required to draw this mesh in its default configuration.
	const OpenGLMesh& getGLMesh(const MeshID& pMeshID) const;
	const GLData::Texture& getTexture(const TextureID& pTextureID) const;
	// Returns the shader needed to execute a particular DrawCall
	Shader* getShader(const DrawCall& pDrawCall);
	// Recursively draw the OpenGLMesh and all its children.
	void draw(const OpenGLMesh& pMesh);

	static GladGLContext* initialiseGLAD(); // Requires a GLFW window to be set as current context, done in OpenGLWindow constructor
	static void windowSizeCallback(GLFWwindow* pWindow, int pWidth, int pHeight); // Callback required by GLFW to be static/global.

	const int cOpenGLVersionMajor, cOpenGLVersionMinor;
	// By default, OpenGL projection uses non-linear depth values (they have a very high precision for small z-values and a low precision for large z-values).
	// By setting this to true, BufferDrawType::Depth will visualise the values in a linear fashion from mZNearPlane to mZFarPlane.
	bool mLinearDepthView;
	bool mVisualiseNormals;
	float mZNearPlane;
	float mZFarPlane;
	float mFOV;

	// The window and GLAD context must be first declared to enforce the correct initialisation order:
	// ***************************************************************************************************
	OpenGLWindow mWindow;		 // GLFW window which both GLFWInput and OpenGLAPI depend on for their construction.
	GladGLContext* mGLADContext; // Depends on OpenGLWindow being initialised first. Must be declared after mWindow.
	GLState mGLState;

	size_t mTexture1ShaderIndex;
	size_t mTexture2ShaderIndex;
	size_t mUniformShaderIndex;
	size_t mMaterialShaderIndex;
	size_t mLightMapIndex;
	size_t mDepthViewerIndex;
	size_t mScreenTextureIndex;
	size_t mSkyBoxShaderIndex;
	size_t mVisualiseNormalIndex;
	MeshID mScreenQuad;
	MeshID mSkyBoxMeshID;
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

	// OpenGL data
	GLData::FBO mMainScreenFBO;
	std::vector<Shader> mShaders;
	std::vector<OpenGLMesh> mGLMeshes;
	std::vector<GLData::Texture> mTextures; // Mapping of Zephyr::Texture to OpenGL::Texture.
	std::vector<GLData::Texture> mCubeMaps;
};