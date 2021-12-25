#pragma once

#include "Context.hpp"
#include "unordered_map"

typedef struct GLFWwindow GLFWwindow;
struct GladGLContext;

// Implements OpenGL API via GLAD2 and binds it to mWindow provided by GLFW.
// GLFW also provides input functionality which is set to callback to static functions keyCallback() and windowSizeCallback()
class OpenGLContext : public Context
{
public:
	OpenGLContext();
	~OpenGLContext();

	// Inherited from Context.hpp
	bool initialise() 																override;
	bool isClosing() 																override;
	void close() 																	override;
	void clearWindow() 																override;
	void swapBuffers() 																override;
	void newImGuiFrame() 															override;
	void renderImGuiFrame() 														override;
	void setClearColour(const float& pRed, const float& pGreen, const float& pBlue) override;

	void draw() 								override;
	void initialiseMesh(const Mesh &pMesh) 		override;

protected:
	bool initialiseImGui() 															override;
	void shutdownImGui() 															override;

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

	void initialiseTextures();
	unsigned int loadTexture(const std::string &pFileName);
	void initialiseShaders();
	unsigned int loadShader(const std::string &pVertexShader, const std::string &pFragmentShader);
	enum class ProgramType
	{
		VertexShader,
		FragmentShader,
		ShaderProgram
	};
	bool hasCompileErrors(const unsigned int pProgramID, const ProgramType &pType);

	int getPolygonMode(const DrawCall::DrawMode& pDrawMode);

	bool createWindow(const char *pName, int pWidth, int pHeight, bool pResizable = true);

	// Callback required by GLFW to be static/global
	static void windowSizeCallback(GLFWwindow *window, int width, int height);


	const int cOpenGLVersionMajor, cOpenGLVersionMinor;
	const std::string cGLSLVersion;
	const size_t cMaxTextureUnits; // The limit on the number of texture units available in the shaders using sampler2D

	unsigned int mRegularShader;
	unsigned int mTextureShader;
	GLFWwindow 		*mWindow;
	GladGLContext 	*mGLADContext;
};