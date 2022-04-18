#pragma once

#include "GraphicsAPI.hpp"

#include "OpenGLWindow.hpp"
#include "GLState.hpp"
#include "Shader.hpp"

#include "Utility.hpp"
#include "Types.hpp"

#include "unordered_map"
#include "optional"
#include "array"
#include "vector"
#include "string"

struct GladGLContext;
struct DrawCall;
struct Mesh;
enum class DrawMode;

class OpenGLAPI : public GraphicsAPI
{
public:
	OpenGLAPI(const LightManager& pLightManager);
	~OpenGLAPI();

	void onFrameStart() 									override;
	void draw(const DrawCall& pDrawCall) 					override;
	void draw(const PointLight& pPointLight) 				override;
	void draw(const DirectionalLight& pDirectionalLight) 	override;
	void draw(const SpotLight& pSpotLight) 					override;
	void postDraw() 										override;

	void newImGuiFrame()	override;
	void renderImGuiFrame() override;
	void renderImGui()		override;

	void initialiseMesh(const Mesh& pMesh) 			override;
	void initialiseTexture(const Texture& pTexture) override;
private:
	struct OpenGLMesh
	{
		enum class DrawMethod { Indices, Array, Null };

		unsigned int mDrawMode = 0;
		int mDrawSize = -1; // Cached size of data used in OpenGL draw call, either size of Mesh positions or indices
		DrawMethod mDrawMethod = DrawMethod::Null;
		std::vector<OpenGLMesh> mChildMeshes;

		GLData::VAO mVAO;
		GLData::EBO mEBO;
		std::array<std::optional<GLData::VBO>, util::toIndex(Shader::Attribute::Count)> mVBOs;
	};

	int getPolygonMode(const DrawMode& pDrawMode);
	// Get all the data required to draw this mesh in its default configuration.
	const OpenGLMesh& getGLMesh(const MeshID& pMeshID) const;
	const GLData::Texture& getTexture(const TextureID& pTextureID) const;
	// Recursively draw the OpenGLMesh and all its children.
	void draw(const OpenGLMesh& pMesh);

	// Pushes the Mesh attribute to a GPU using a VBO. Returns the VBO generated.
	template <class T>
	std::optional<GLData::VBO> bufferAttributeData(const std::vector<T> &pData, const Shader::Attribute& pAttribute);

	static GladGLContext* initialiseGLAD(); // Requires a GLFW window to be set as current context, done in OpenGLWindow constructor
	static void windowSizeCallback(GLFWwindow* pWindow, int pWidth, int pHeight); // Callback required by GLFW to be static/global.

	const int cOpenGLVersionMajor, cOpenGLVersionMinor;
	// By default, OpenGL projection uses non-linear depth values (they have a very high precision for small z-values and a low precision for large z-values).
	// By setting this to true, BufferDrawType::Depth will visualise the values in a linear fashion from mZNearPlane to mZFarPlane.
	bool mLinearDepthView;
	GLType::BufferDrawType BufferDrawType;
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
	TextureID mMissingTextureID;
	int pointLightDrawCount;
	int spotLightDrawCount;
	int directionalLightDrawCount;

	GLType::BufferDrawType mBufferDrawType;

	std::vector<Shader> mShaders;
	// Draw info is fetched every draw call.
	// @PERFORMANCE We should store OpenGLMesh on the stack for faster access.
	std::unordered_map<MeshID, OpenGLMesh> mGLMeshes;
	std::array<GLData::Texture, MAX_TEXTURES> mTextures; // Mapping of Zephyr::Texture to OpenGL::Texture.

};