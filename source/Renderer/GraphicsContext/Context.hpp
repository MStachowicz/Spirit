#pragma once

#include <vector>

struct Mesh;

// Context is an interface for specific graphics API's to implement.
// Renderer then uses the interface to call the correct implementation
// based on the API selected at generation time.
class Context
{
public:
	virtual bool initialise()	= 0;
	virtual bool isClosing() 	= 0;
	virtual void close() 		= 0;
	virtual void clearWindow() 	= 0;
	virtual void swapBuffers() 	= 0;
	virtual void pollEvents() 	= 0;

	virtual void draw(const Mesh& pMesh) = 0;
	virtual void setHandle(Mesh& pMesh) = 0;

	virtual void setClearColour(const float& pRed, const float& pGreen, const float& pBlue) = 0;

	virtual void newImGuiFrame() = 0;
	virtual void renderImGuiFrame() = 0;

protected:
	virtual bool initialiseImGui() 	= 0; // Because ImGui backend depends on the API used - we need to initialise it as part of the GraphicsContext
	virtual void shutdownImGui() 	= 0;
};

// Mesh provides the vertex information and draw specifications the API implemented will use to render on screen.
// Mesh initialisation requires some vertices to be added before calling Context::setHandle(Mesh) to assign its mVAO handle.
// mDrawMode and mPolygon mode are runtime draw options that take effect at the Context::draw(Mesh) call.
struct Mesh
{
	// Draw modes map directly to OpenGL draw modes found in gl.h
	enum class DrawMode : unsigned int
	{
		Triangles = 0x0004,
		Unknown = 0 // The default (un-initialised) drawMode, this is set in Context::setHandle(Mesh).
	};

	// Polygon modes map directly to OpenGL polygon modes found in gl.h
	enum class PolygonMode : unsigned int
	{
		Fill = 0x1B02,
		Wireframe = 0x1B01
	};

	unsigned int mVAO = 0;
	std::vector<float> mVertices;	// Per-vertex position attributes.
	std::vector<float> mColours;	// Per-vertex colour attributes.
	std::vector<int>   mIndices; // Allows indexing into the mVertices and mColours data to specify an indexed draw order.

	DrawMode 	mDrawMode 		= DrawMode::Unknown;
	PolygonMode mPolygonMode 	= PolygonMode::Fill;
};

// This function is useful for converting enums to their underlying types such as DrawMode for setting up draw calls with its underlying value.
// In C++ 23 std::to_underlying can be used instead.
// Source: https://stackoverflow.com/questions/14589417/can-an-enum-class-be-converted-to-the-underlying-type
template <typename T>
constexpr auto castToUnderlyingType(T pEnumObject)
{
	return static_cast<std::underlying_type_t<T>>(pEnumObject);
}