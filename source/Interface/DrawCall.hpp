#pragma once

#include "Utility.hpp"
#include "Types.hpp"

#include "Mesh.hpp"

#include "glm/vec3.hpp"	// vec3, bvec3, dvec3, ivec3 and uvec3
#include "glm/mat4x4.hpp"

#include <optional>
#include <array>
#include <vector>

// A request to execute a specific draw using a GraphicsAPI.
struct DrawCall
{
	std::vector<glm::mat4> mModels;
	Data::MeshDraw mMesh;
};