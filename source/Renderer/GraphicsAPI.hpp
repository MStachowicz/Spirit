#pragma once

#include "Logger.hpp"

#include "Mesh.hpp"
#include "Texture.hpp"

#include "DrawCall.hpp"
#include "LightManager.hpp"

#include "glm/vec3.hpp"	// vec3, bvec3, dvec3, ivec3 and uvec3
#include "glm/mat4x4.hpp" // mat4, dmat4

#include "vector"
#include "string"
#include "optional"

class MeshManager;

// GraphicsAPI is an interface for Zephyr::Renderer to communicate with a derived Graphics pipeline.
// A derived API must empty it's mDrawQueue in it's draw function.
class GraphicsAPI
{
public:
	GraphicsAPI(const MeshManager& pMeshManager, const TextureManager& pTextureManager)
	: mMeshManager(pMeshManager)
	, mTextureManager(pTextureManager)
	{};

	virtual ~GraphicsAPI() {}; // Context is an pure virtual interface used polymorphically.

	virtual void onFrameStart() 					= 0;
	virtual void draw(const DrawCall& pDrawCall) 	= 0; // Executes the draw call.
	virtual void postDraw() 						= 0;

	void setView(const glm::mat4& pViewMatrix) { mViewMatrix = pViewMatrix; }
	void setViewPosition(const glm::vec3& pViewPosition) { mViewPosition = pViewPosition; }
protected:
	// Sets up the mesh for processing DrawCalls from the mDrawQueue queue.
	virtual void initialiseMesh(const Mesh& pMesh) = 0;
	virtual void initialiseTexture(const Texture& pTexture) = 0;

	glm::mat4 mViewMatrix; // The view matrix used in draw(), set in setView
	glm::vec3 mViewPosition; // The view position used in draw(), set in setViewPosition

	const MeshManager& mMeshManager; // Owned by Zephyr::Renderer.
	const TextureManager& mTextureManager; // Owned by Zephyr::Renderer.
	LightManager mLightManager;
};