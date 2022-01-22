#pragma once

#include "GraphicsAPI.hpp"

#include "OpenGLWindow.hpp"
#include "Shader.hpp"

#include "glm/mat4x4.hpp" // mat4, dmat4
#include "unordered_map"
#include <array>

struct GladGLContext;

// Implements GraphicsAPI. Takes DrawCalls and clears them in draw() using an OpenGL rendering pipeline.
class OpenGLAPI : public GraphicsAPI
{
public:
	OpenGLAPI();
	~OpenGLAPI();

	// Initialising OpenGLAPI requires an OpenGLWindow to be created beforehand as GLAD requires a context to be set for its initialisation.
	void draw() 								override;
	void onFrameStart() 						override;
	void setView(const glm::mat4& pViewMatrix)	override;
private:
	void initialiseMesh(const Mesh &pMesh) 		override;
	void initialiseTextures();
	unsigned int loadTexture(const std::string &pFileName);

	void setClearColour(const float& pRed, const float& pGreen, const float& pBlue);
	void clearBuffers();
	int getPolygonMode(const DrawCall::DrawMode& pDrawMode);

	const int cOpenGLVersionMajor, cOpenGLVersionMinor;
	float mWindowClearColour[3]; // Colour the window will be cleared with in RGB 0-1.
	glm::mat4 mViewMatrix; // Cached view matrix the active camera has been set to. Updated via callback using setView()

	// The window and GLAD context must be first declared to enforce the correct initialisation order:
	// ***************************************************************************************************
	OpenGLWindow mWindow;		 // GLFW window which both GLFWInput and OpenGLAPI depend on for their construction.
	GladGLContext* mGLADContext; // Depends on OpenGLWindow being initialised first. Must be declared after mWindow.

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

	// Responsible for all the data this instance of OpenGLAPI has pushed to the GPU.
	// Also stores information relevant to how a mesh should be drawn by default.
	// All data is stored on a per Mesh basis.
	class GPUDataManager
	{
	public:
		// Bind this Mesh's VAO as the current in use.
		void bindVAO(const MeshID& pMeshID);
		// Get all the data required to draw this mesh in its default configuration.
		const DrawInfo& getDrawInfo(const MeshID& pMeshID);
		// Load the mesh into OpenGL by pushing all its available data to the GPU and assigning it a VAO to draw from.
		void loadMesh(const Mesh& pMesh);

		std::vector<Shader> mShaders;
		// Draw info is fetched every draw call.
		// @PERFORMANCE We should store DrawInfo on the stack for faster access.
		std::unordered_map<MeshID, DrawInfo> mDrawInfos;
	private:
		// Pushes the Mesh attribute to a GPU using a VBO. Returns the VBO handle.
		template <class T>
		unsigned int bufferAttributeData(const std::vector<T>& pData, const Shader::Attribute& pAttribute);
		void assignVAO(const MeshID& pMeshID);
		void assignDrawInfo(const MeshID& pMeshID, const DrawInfo& pDrawInfo);

		static bool isMeshValidForShader(const Mesh& pMesh, const Shader& pShader);

		// VBO represents some data pushed to the GPU.
		// On destruction of VBO the data on the GPU is freed.
		// Because of the above, VBO lifetime should be handled with dynamic memory and be wrapped as in
		struct VBO
		{
			VBO(unsigned int pHandle) : mHandle(pHandle)
			{}
			~VBO();

			// VBO is a move only type, we don't want to de-allocate the VBO handle on copy. Delete all copy operators.
			VBO(VBO &&other) 		 			= default; 	// move constructor
			VBO &operator=(VBO &&) 				= default;	// move assignment
			VBO(const VBO &) 					= delete; 	// copy constructor
			VBO &operator=(const VBO &) 		= delete; 	// copy assignment

		private:
			unsigned int mHandle;	// A mesh can push multiple attributes onto the GPU.
		};
		struct VAO
		{
			VAO();
			~VAO();

			void bind() const;

			// VBO is a move only type, we don't want to de-allocate the VBO handle on copy. Delete all copy operators.
			VAO(VAO &&other) 		 			= default; 	// move constructor
			VAO &operator=(VAO &&) 				= default;	// move assignment
			VAO(const VAO &) 					= delete; 	// copy constructor
			VAO &operator=(const VAO &) 		= delete; 	// copy assignment

		private:
			unsigned int mHandle;	// A mesh can push multiple attributes onto the GPU.
		};

		std::unordered_map<MeshID, std::unique_ptr<VAO>> mVAOs;
		std::unordered_map<MeshID, std::array<std::unique_ptr<VBO>, Shader::toIndex(Shader::Attribute::Count)>> mVBOs;

	};
	GPUDataManager mDataManager;

	static GladGLContext* initialiseGLAD(); // Requires a GLFW window to be set as current context, done in OpenGLWindow constructor
	static void windowSizeCallback(GLFWwindow *pWindow, int pWidth, int pHeight); // Callback required by GLFW to be static/global.
};