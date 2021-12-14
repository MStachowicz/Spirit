#pragma once

#include "Context.hpp"
#include "unordered_map"

typedef struct GLFWwindow GLFWwindow;
struct GladGLContext;

// Implements OpenGL API via GLAD2 and binds it to mWindow provided by GLFW.
// GLFW also provides input functionality which is set to callback to static functions
// keyCallback() and windowSizeCallback()
class OpenGLContext : public Context
{
public:
	OpenGLContext();
	~OpenGLContext();

	// Inherited from Context.hpp
	bool initialise() 	override;
	bool isClosing() 	override;
	void close() 		override;
	void clearWindow() 	override;
	void swapBuffers() 	override;
	void pollEvents() 	override;

	void draw(const Mesh &pMesh) override;
	void setHandle(Mesh &pMesh) override;

	void setClearColour(const float& pRed, const float& pGreen, const float& pBlue) override;
	void newImGuiFrame() override;
	void renderImGuiFrame() override;

protected:
	bool initialiseImGui() 	override;
	void shutdownImGui() 	override;

private:
	enum class ProgramType
	{
		VertexShader,
		FragmentShader,
		ShaderProgram
	};

	// All the data representing HOW a mesh should be rendered. While all the Mesh data
	// is stored in Context::Mesh, this object identifies how OpenGL should draw this mesh.
	// This is set in the setHandle() function
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
		unsigned int mShaderID = invalidHandle;
		unsigned int mVAO = invalidHandle;
		unsigned int mVBO = invalidHandle;
		unsigned int mEBO = invalidHandle;
		unsigned int mDrawMode = invalidHandle;	   // Draw modes map directly to OpenGL draw modes found in gl.h
		unsigned int mPolygonMode = invalidHandle; // Polygon modes map directly to OpenGL polygon modes found in gl.h
		size_t mDrawSize = invalidHandle;		   // Cached size of data used in draw, either size of Mesh positions or indices
		DrawMethod mDrawMethod = DrawMethod::Null;
		std::vector<unsigned int> mTextures; // The textures to bind before calling draw.
	};
	std::unordered_map<MeshID, DrawInfo> mMeshManager; // Mapping of a Mesh to how it should be drawn.
	std::unordered_map<std::string, unsigned int> mTextures;

	// TODO assign these using an environment variable (OpenGL version set in glad_add_library() in CMakeLists.txt)
	const int cOpenGLVersionMajor, cOpenGLVersionMinor;
	const std::string cGLSLVersion;
	const size_t cMaxTextureUnits; // The limit on the number of texture units available in the shaders using sampler2D

	unsigned int mShaderProgram;
	unsigned int mTextureShader;
	GLFWwindow *mWindow;
	GladGLContext *mGLADContext;

	void initialiseShaders();
	void initialiseTextures();

	bool hasCompileErrors(const unsigned int pProgramID, const ProgramType &pType);
	bool createWindow(const char *pName, int pWidth, int pHeight, bool pResizable = true);
	unsigned int loadTexture(const std::string &pFileName);
	unsigned int loadShader(const std::string &pVertexShader, const std::string &pFragmentShader);

	void draw(const DrawInfo &pDrawInfo);

	// Callbacks required by GLFW to be static/global
	static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
	static void windowSizeCallback(GLFWwindow *window, int width, int height);
};