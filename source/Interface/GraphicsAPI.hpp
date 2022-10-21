#pragma once

#include "glm/vec3.hpp"	// vec3, bvec3, dvec3, ivec3 and uvec3
#include "glm/mat4x4.hpp" // mat4, dmat4

#include "DrawCall.hpp"

struct Texture;
struct CubeMapTexture;

namespace ECS
{
	class Entity;
	class EntityManager;
}
namespace Data
{
	struct Transform;
	struct Mesh;
	struct PointLight;
	struct DirectionalLight;
	struct SpotLight;
}

// GraphicsAPI implements behaviour all derived GraphicsAPI's depend on.
// Namely it enforces an interface used by Zephyr::Renderer for executing DrawCalls.
// It is also a listener to ECS data changes to relevant components to DrawCalls.
class GraphicsAPI
{
public:
	GraphicsAPI()
	{};
	virtual ~GraphicsAPI() {}; // GraphicsAPI is a pure virtual interface used polymorphically.

	virtual void preDraw() 											   = 0;
	virtual void draw()												   = 0;
	virtual void postDraw() 										   = 0;
	virtual void endFrame()											   = 0;

	virtual void setupLights(const bool& pRenderLightPositions)		   = 0;

	virtual void newImGuiFrame() 									   = 0;
	virtual void renderImGuiFrame() 								   = 0;
	virtual void renderImGui() {};

	virtual void initialiseMesh(const Data::Mesh& pMesh)           	   = 0;
	virtual void initialiseTexture(const Texture& pTexture)        	   = 0;
	virtual void initialiseCubeMap(const CubeMapTexture& pCubeMap) 	   = 0;

	void setView(const glm::mat4& pViewMatrix) { mViewMatrix = pViewMatrix; }
	void setViewPosition(const glm::vec3& pViewPosition) { mViewPosition = pViewPosition; }

	virtual void onEntityCreated(const ECS::Entity& pEntity, const ECS::EntityManager& pManager);
	virtual void onEntityRemoved(const ECS::Entity& pEntity, const ECS::EntityManager& pManager);
	virtual void onTransformComponentAdded(const ECS::Entity& pEntity, const Data::Transform& pTransform);
	virtual void onTransformComponentChanged(const ECS::Entity& pEntity, const Data::Transform& pTransform);
	virtual void onTransformComponentRemoved(const ECS::Entity& pEntity);
	virtual void onMeshComponentAdded(const ECS::Entity& pEntity, const Data::MeshDraw& pMeshDraw);
	virtual void onMeshComponentChanged(const ECS::Entity& pEntity, const Data::MeshDraw& pMeshDraw);
	virtual void onMeshComponentRemoved(const ECS::Entity& pEntity);

protected:
	std::vector<DrawCall> mDrawCalls; // GraphicsAPI Executes all these DrawCalls using the draw function.

	glm::mat4 mViewMatrix; // The view matrix used in draw(), set in setView
	glm::vec3 mViewPosition; // The view position used in draw(), set in setViewPosition
	glm::mat4 mProjection;
};