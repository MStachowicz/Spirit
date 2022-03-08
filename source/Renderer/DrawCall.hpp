#pragma once

#include "Mesh.hpp"

#include "glm/vec3.hpp"	// vec3, bvec3, dvec3, ivec3 and uvec3

// A request to execute a specific draw using a GraphicsAPI.
struct DrawCall
{
	enum class DrawMode
	{
		Fill,
		Wireframe
	};

	MeshID 		mMesh;
	DrawMode 	mDrawMode  	= DrawMode::Fill;
	glm::vec3 	mPosition 	= glm::vec3(0.0f);
	glm::vec3 	mRotation	= glm::vec3(0.0f);
	glm::vec3 	mScale		= glm::vec3(1.0f);
};