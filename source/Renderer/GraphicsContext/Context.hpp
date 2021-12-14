#pragma once

#include <vector>
#include <string>

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

// The unique ID the mesh uses to identify its draw information in a specific draw context.
typedef unsigned int MeshID;
static MeshID nextMesh = 0;
inline MeshID getMeshID()
{
	return ++nextMesh;
}
typedef unsigned int TextureID;

// Mesh stores all the vertex (and optionally index) data that a graphics API will use to assign its internal DrawInfo using Context::setHandle.
// Once initialised the unique mID field will map to the DrawInfo in the context.
struct Mesh
{
	Mesh() : mID(getMeshID())
	{}

	const MeshID mID;									// Unique ID to map this mesh to DrawInfo within the graphics context being used.
	std::vector<float> 			mVertices;				// Per-vertex position attributes.
	std::vector<float> 			mColours;				// Per-vertex colour attributes.
	std::vector<float> 			mTextureCoordinates; 	// Per-vertex texture mapping.
	std::vector<std::string> 	mTextures;				// The filenames of the textures this mesh is using.
	std::vector<int>   			mIndices; 				// Allows indexing into the mVertices and mColours data to specify an indexed draw order.
};