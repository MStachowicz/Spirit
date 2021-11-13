#include "Context.hpp"

// Implements OpenGL API and a window using GLAD
// and GLFW respectively.
class OpenGLContext : public Context
{
public:
	~OpenGLContext();

	// Inherited from Context.hpp
	bool initialise() 	override;
	bool isClosing() 	override;
	void close() 		override;
	void clearWindow() 	override;
	void swapBuffers() 	override;
	void pollEvents() 	override;
private:
	// OpenGL version set in glad_add_library() in CMakeLists.txt
	const int cOpenGLVersionMajor = 3, cOpenGLVersionMinor = 3;

	void shutdown();
	bool createWindow(const char *pName, int pWidth, int pHeight, bool pResizable = true);
	void setClearColour(float pRed, float pGreen, float pBlue);
};