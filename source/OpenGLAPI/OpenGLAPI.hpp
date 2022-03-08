#pragma once

#include "GraphicsAPI.hpp"

#include "OpenGLWindow.hpp"
#include "Shader.hpp"

#include "unordered_map"
#include <array>

struct GladGLContext;

// Implements GraphicsAPI. Takes DrawCalls and clears them in draw() using an OpenGL rendering pipeline.
class OpenGLAPI : public GraphicsAPI
{
public:
	OpenGLAPI(const MeshManager& pMeshManager, const TextureManager& pTextureManager);
	~OpenGLAPI();

	void onFrameStart() 						override;
	void draw(const DrawCall& pDrawCall) 		override;
	void postDraw() 							override;

private:
	void initialiseMesh(const Mesh& pMesh) 			override;
	void initialiseTexture(const Texture& pTexture) override;

	void setClearColour(const float& pRed, const float& pGreen, const float& pBlue);
	void clearBuffers();
	int getPolygonMode(const DrawCall::DrawMode& pDrawMode);

	const int cOpenGLVersionMajor, cOpenGLVersionMinor;
	float mWindowClearColour[3]; // Colour the window will be cleared with in RGB 0-1.

	// The window and GLAD context must be first declared to enforce the correct initialisation order:
	// ***************************************************************************************************
	OpenGLWindow mWindow;		 // GLFW window which both GLFWInput and OpenGLAPI depend on for their construction.
	GladGLContext* mGLADContext; // Depends on OpenGLWindow being initialised first. Must be declared after mWindow.
	std::vector<Shader> mShaders;

	// Defines HOW a Mesh should be rendered, has a 1:1 relationship with mesh
	struct DrawInfo
	{
		enum class DrawMethod{ Indices, Array, Null };
		DrawInfo();

		const Shader* activeShader; // Shader used to draw
		std::vector<const Shader*> mShadersAvailable; // All the available shaders that can be drawn with.
		unsigned int mEBO;
		unsigned int mDrawMode;
		int mDrawSize; // Cached size of data used in draw, either size of Mesh positions or indices
		DrawMethod mDrawMethod = DrawMethod::Null;

		static const unsigned int invalidHandle = 0;
	};
	// VBO represents some data pushed to the GPU.
	// VBO::release needs to be called before destruction to free the associated memory.
	struct VBO
	{
		VBO(unsigned int pHandle) : mHandle(pHandle)
		{}
		void release();

	private:
		unsigned int mHandle; // A mesh can push multiple attributes onto the GPU.
	};
	struct VAO
	{
		VAO();
		void bind() const;
		void release();
	private:
		unsigned int mHandle; // A mesh can push multiple attributes onto the GPU.
	};

	// Get all the data required to draw this mesh in its default configuration.
	const DrawInfo& getDrawInfo(const MeshID &pMeshID);
	const VAO& getVAO(const MeshID& pMeshID);

	// Pushes the Mesh attribute to a GPU using a VBO. Returns the VBO generated.
	template <class T>
	std::optional<VBO> bufferAttributeData(const std::vector<T> &pData, const Shader::Attribute &pAttribute);

	// Draw info is fetched every draw call.
	// @PERFORMANCE We should store DrawInfo on the stack for faster access.
	std::unordered_map<MeshID, DrawInfo> mDrawInfos;
	std::unordered_map<MeshID, VAO> mVAOs;
	std::unordered_map<MeshID, std::array<std::optional<VBO>, Shader::toIndex(Shader::Attribute::Count)>> mVBOs;

	static GladGLContext* initialiseGLAD(); // Requires a GLFW window to be set as current context, done in OpenGLWindow constructor
	static void windowSizeCallback(GLFWwindow *pWindow, int pWidth, int pHeight); // Callback required by GLFW to be static/global.
	static bool isMeshValidForShader(const Mesh &pMesh, const Shader &pShader);
};