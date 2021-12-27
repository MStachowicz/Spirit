#pragma once

#include "Context.hpp"
#include "unordered_map"
#include "OpenGLWindow.hpp"

struct GladGLContext;

// Implements OpenGL API via GLAD2 and binds it to mWindow provided by GLFW.
// Implements the Context API used by Renderer taking DrawCalls and executing draw().
class OpenGLContext : public Context
{
public:
	OpenGLContext();
	~OpenGLContext();

	// Initialising OpenGLContext requires an OpenGLWindow to be created beforehand as GLAD requires a context to be set for its initialisation.
	bool initialise() 																	override;
	void setClearColour(const float& pRed, const float& pGreen, const float& pBlue) 	override;
	void draw() 																		override;
	void initialiseMesh(const Mesh &pMesh) 												override;
	void onFrameStart() 																override;
private:
	// Defines HOW a Mesh should be rendered, has a 1:1 relationship with mesh
	struct DrawInfo
	{
		static const unsigned int invalidHandle = 0;
		enum class DrawMethod
		{
			Indices,
			Array,
			Null
		};

		// OpenGL handles
		unsigned int mShaderID 		= invalidHandle;
		unsigned int mVAO 			= invalidHandle;
		unsigned int mVBO 			= invalidHandle;
		unsigned int mEBO 			= invalidHandle;
		unsigned int mDrawMode 		= invalidHandle;
		size_t mDrawSize 			= invalidHandle; // Cached size of data used in draw, either size of Mesh positions or indices
		DrawMethod mDrawMethod 		= DrawMethod::Null;
	};
	const DrawInfo& getDrawInfo(const MeshID& pMeshID);
	std::unordered_map<MeshID, DrawInfo> mMeshManager; 		// Mapping of a Mesh to how it should be drawn.

	void clearBuffers();

	void initialiseTextures();
	void initialiseShaders();
	unsigned int loadTexture(const std::string &pFileName);
	unsigned int loadShader(const std::string &pVertexShader, const std::string &pFragmentShader);
	enum class ProgramType
	{
		VertexShader,
		FragmentShader,
		ShaderProgram
	};
	bool hasCompileErrors(const unsigned int pProgramID, const ProgramType &pType);
	int getPolygonMode(const DrawCall::DrawMode& pDrawMode);

	const int cOpenGLVersionMajor, cOpenGLVersionMinor;
	const size_t cMaxTextureUnits; // The limit on the number of texture units available in the shaders using sampler2D
	unsigned int mRegularShader;
	unsigned int mTextureShader;
	GladGLContext *mGLADContext;
	OpenGLWindow mWindow;

	static void windowSizeCallback(GLFWwindow *pWindow, int pWidth, int pHeight); // Callback required by GLFW to be static/global.
};