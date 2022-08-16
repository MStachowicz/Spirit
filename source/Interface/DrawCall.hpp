#pragma once

// Data
#include "Mesh.hpp"

// ECS
#include "Entity.hpp"

// GLM
#include "glm/mat4x4.hpp"

// STD
#include <vector>
#include <unordered_map>

// A request to execute a specific MeshDraw at a number of locations using a GraphicsAPI.
// DrawCall's purpose is to group together the same MeshDraw's differentiating them only by the Model matrices found inside mModels.
// For the above reason DrawCalls are a Zephyr::Renderer construct as they are fed to GraphicsAPI's in this more parsable format for instancing.
struct DrawCall
{
	Data::MeshDraw mMesh;

	// List of per-Entity transform matrices
	std::vector<glm::mat4> mModels;
	// Mapping of EntityID to index into mModels.
	std::unordered_map<ECS::EntityID, size_t> mEntityModelIndexLookup;
};