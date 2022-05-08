#pragma once

#include "glm/vec3.hpp"	// vec3, bvec3, dvec3, ivec3 and uvec3
#include "glm/mat4x4.hpp" // mat4, dmat4

class MeshManager;
class TextureManager;
class LightManager;
struct Mesh;
struct Texture;
struct DrawCall;
struct PointLight;
struct DirectionalLight;
struct SpotLight;

// GraphicsAPI is an interface for Zephyr::Renderer to communicate with a derived Graphics pipeline.
class GraphicsAPI
{
public:
	GraphicsAPI(const LightManager& pLightManager)
	: mLightManager(pLightManager)
	{};
	virtual ~GraphicsAPI() {}; // GraphicsAPI is a pure virtual interface used polymorphically.

	virtual void preDraw() 											= 0;
	virtual void draw(const DrawCall& pDrawCall) 					= 0;
	virtual void draw(const PointLight& pPointLight) 				= 0;
	virtual void draw(const DirectionalLight& pDirectionalLight) 	= 0;
	virtual void draw(const SpotLight& pSpotLight) 					= 0;
	virtual void postDraw() 										= 0;
	virtual void endFrame()											= 0;

	virtual void newImGuiFrame() 	= 0;
	virtual void renderImGuiFrame() = 0;
	virtual void renderImGui() {};

	virtual void initialiseMesh(const Mesh& pMesh) = 0;
	virtual void initialiseTexture(const Texture& pTexture) = 0;

	void setView(const glm::mat4& pViewMatrix) { mViewMatrix = pViewMatrix; }
	void setViewPosition(const glm::vec3& pViewPosition) { mViewPosition = pViewPosition; }
protected:

	glm::mat4 mViewMatrix; // The view matrix used in draw(), set in setView
	glm::vec3 mViewPosition; // The view position used in draw(), set in setViewPosition
	glm::mat4 mProjection;

	const LightManager& mLightManager;
};