#pragma once

#include "OpenGLWindow.hpp"
#include "GraphicsAPI.hpp"

#include "unordered_map"
#include "glm/mat4x4.hpp" // mat4, dmat4

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
	const size_t cMaxTextureUnits; // The limit on the number of texture units available in the shaders using sampler2D
	float mWindowClearColour[3]; // Colour the window will be cleared with in RGB 0-1.
	glm::mat4 mViewMatrix; // Cached view matrix the active camera has been set to. Updated via callback using setView()

	OpenGLWindow mWindow;
	GladGLContext* mGLADContext; // Depends on OpenGLWindow being initialised first. Must be declared after mWindow.

	struct Shader
	{
		Shader(const std::string& pName);
		void use(); // Sets this as the active shader for the OpenGL state
		int getAttributeLocation(const std::string &pName);

		void setUniform(const std::string &pName, const bool& pValue);
		void setUniform(const std::string &pName, const int& pValue);
		void setUniform(const std::string &pName, const glm::mat4& pValue);
	private:
		int getUniformLocation(const std::string &pName);
		void load();

		std::string mName;
		std::string mSourcePath;
		unsigned int mHandle;

		enum class Type { Vertex, Fragment, Program };
		static bool hasCompileErrors(const Type& pType, const unsigned int pID);
	};
	Shader mTextureShader;

	// Defines HOW a Mesh should be rendered, has a 1:1 relationship with mesh
	struct DrawInfo
	{
		DrawInfo(Shader& pShader);

		Shader& mShaderID; // Shader used to make this draw call.
		unsigned int mVAO;
		unsigned int mVBO;
		unsigned int mEBO;
		unsigned int mDrawMode;
		int mDrawSize; // Cached size of data used in draw, either size of Mesh positions or indices

		enum class DrawMethod{ Indices, Array, Null };
		DrawMethod mDrawMethod 		= DrawMethod::Null;

	private:
		static const unsigned int invalidHandle = 0;
	};
	const DrawInfo& getDrawInfo(const MeshID& pMeshID);
	std::unordered_map<MeshID, DrawInfo> mMeshManager; 		// Mapping of a Mesh to how it should be drawn.

	static GladGLContext* initialiseGLAD(); // Requires a GLFW window to be set as current context, done in OpenGLWindow constructor
	static void windowSizeCallback(GLFWwindow *pWindow, int pWidth, int pHeight); // Callback required by GLFW to be static/global.
};