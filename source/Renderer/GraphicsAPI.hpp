#pragma once

#include "glm/vec3.hpp"	// vec3, bvec3, dvec3, ivec3 and uvec3
#include "glm/mat4x4.hpp" // mat4, dmat4

class MeshManager;
class TextureManager;
class LightManager;
struct Mesh;
struct Texture;
struct DrawCall;

// GraphicsAPI is an interface for Zephyr::Renderer to communicate with a derived Graphics pipeline.
class GraphicsAPI
{
public:
	GraphicsAPI(const MeshManager& pMeshManager, const TextureManager& pTextureManager, const LightManager& pLightManager)
	: mMeshManager(pMeshManager)
	, mTextureManager(pTextureManager)
	, mLightManager(pLightManager)
	{};

	virtual ~GraphicsAPI() {}; // GraphicsAPI is a pure virtual interface used polymorphically.

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
	glm::mat4 mProjection;

	const MeshManager& mMeshManager; // Owned by Zephyr::Renderer.
	const TextureManager& mTextureManager; // Owned by Zephyr::Renderer.
	const LightManager& mLightManager;
};