#pragma once

#include "Mesh.hpp"

#include "Entity.hpp"

#include "glm/mat4x4.hpp"

#include <vector>
#include <unordered_map>

// A request to execute a specific draw using a GraphicsAPI.
struct DrawCall
{
	Data::MeshDraw mMesh;

	// List of per-Entity transform matrices
	std::vector<glm::mat4> mModels;
	// Mapping of EntityID to index into mModels.
	std::unordered_map<ECS::EntityID, size_t> mEntityModelIndexLookup;
};