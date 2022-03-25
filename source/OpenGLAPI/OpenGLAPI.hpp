#pragma once

#include "GraphicsAPI.hpp"

#include "OpenGLWindow.hpp"
#include "Shader.hpp"

#include "Mesh.hpp"
#include "TextureManager.hpp"
#include "Utility.hpp"

#include "unordered_map"
#include "optional"
#include "array"
#include "vector"

struct GladGLContext;
struct DrawCall;
enum class DrawMode;

class OpenGLAPI : public GraphicsAPI
{
public:
	OpenGLAPI(const MeshManager& pMeshManager, const TextureManager& pTextureManager, const LightManager& pLightManager);
	~OpenGLAPI();

	void onFrameStart() 						override;
	void draw(const DrawCall& pDrawCall) 		override;
	void postDraw() 							override;

private:
	void initialiseMesh(const Mesh& pMesh) 			override;
	void initialiseTexture(const Texture& pTexture) override;

	void setClearColour(const float& pRed, const float& pGreen, const float& pBlue);
	void setDepthTest(const bool& pDepthTest);
	void clearBuffers();
	int getPolygonMode(const DrawMode& pDrawMode);

	const int cOpenGLVersionMajor, cOpenGLVersionMinor;
	float mWindowClearColour[3]; // Colour the window will be cleared with in RGB 0-1.
	bool mDepthTest;
	float mZNearPlane;
	float mZFarPlane;
	float mFOV;

	// The window and GLAD context must be first declared to enforce the correct initialisation order:
	// ***************************************************************************************************
	OpenGLWindow mWindow;		 // GLFW window which both GLFWInput and OpenGLAPI depend on for their construction.
	GladGLContext* mGLADContext; // Depends on OpenGLWindow being initialised first. Must be declared after mWindow.

	size_t mTexture1ShaderIndex;
	size_t mTexture2ShaderIndex;
	size_t mUniformShaderIndex;
	size_t mMaterialShaderIndex;
	size_t mLightMapIndex;

	std::vector<Shader> mShaders;

	// VBO represents some data pushed to the GPU.
	// VBO::release needs to be called before destruction to free the associated memory.
	struct VBO
	{
		void generate();
		void bind() const;
		void release();
	private:
		bool mInitialised 		= false;
		unsigned int mHandle 	= 0;
	};
	struct VAO
	{
		void generate();
		void bind() const;
		void release();
		unsigned int getHandle() { return mHandle; };

	private:
		bool mInitialised 		= false;
		unsigned int mHandle 	= 0;
	};
	struct EBO
	{
		void generate();
		void bind() const;
		void release();
		unsigned int getHandle() { return mHandle; };

	private:
		bool mInitialised 		= false;
		unsigned int mHandle 	= 0;
	};
	struct OpenGLMesh
	{
		enum class DrawMethod{ Indices, Array, Null };

		unsigned int mDrawMode 			= 0;
		int mDrawSize 					= -1; // Cached size of data used in OpenGL draw call, either size of Mesh positions or indices
		DrawMethod mDrawMethod 			= DrawMethod::Null;
		std::vector<OpenGLMesh> mChildMeshes;

		VAO mVAO;
		EBO mEBO;
		std::array<std::optional<VBO>, util::toIndex(Shader::Attribute::Count)> mVBOs;
	};

	// Get all the data required to draw this mesh in its default configuration.
	const OpenGLMesh& getGLMesh(const MeshID& pMeshID);
	// Recursively draw the OpenGLMesh and all its children.
	void draw(const OpenGLMesh& pMesh);
	// Draw info is fetched every draw call.
	// @PERFORMANCE We should store OpenGLMesh on the stack for faster access.
	std::unordered_map<MeshID, OpenGLMesh> mGLMeshes;

	typedef unsigned int TextureHandle;
	TextureHandle getTextureHandle(const TextureID& pTextureID) const;
	std::array<TextureHandle, TextureManager::MAX_TEXTURES> mTextures; // Mapping of Zephyr::Texture to OpenGL::Texture.

	// Pushes the Mesh attribute to a GPU using a VBO. Returns the VBO generated.
	template <class T>
	std::optional<VBO> bufferAttributeData(const std::vector<T> &pData, const Shader::Attribute& pAttribute);

	static GladGLContext* initialiseGLAD(); // Requires a GLFW window to be set as current context, done in OpenGLWindow constructor
	static void windowSizeCallback(GLFWwindow* pWindow, int pWidth, int pHeight); // Callback required by GLFW to be static/global.
};