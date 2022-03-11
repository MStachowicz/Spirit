#pragma once

#include "Mesh.hpp"
#include "Texture.hpp"

#include "glm/vec3.hpp"	// vec3, bvec3, dvec3, ivec3 and uvec3

#include "optional"

enum class DrawMode
{
	Fill,
	Wireframe
};

enum class DrawStyle
{
	Textured,
	UniformColour,
	Default,
	Count
};

// A request to execute a specific draw using a GraphicsAPI.
struct DrawCall
{
	MeshID 					 mMesh;
	std::optional<TextureID> mTexture;

	DrawMode 	mDrawMode  	= DrawMode::Fill;
	DrawStyle	mDrawStyle  = DrawStyle::Default;

	glm::vec3 	mPosition 	= glm::vec3(0.0f);
	glm::vec3 	mRotation	= glm::vec3(0.0f);
	glm::vec3 	mScale		= glm::vec3(1.0f);
};

static std::string convert(const DrawStyle& pDrawStyle)
{
	switch (pDrawStyle)
	{
	case DrawStyle::Default: return "Default";
	case DrawStyle::Textured: return "Textured";
	case DrawStyle::UniformColour : return "Uniform colour";
	default:
		ZEPHYR_ASSERT(false, "No conversion for pDrawStyle");
		return "";
	}
}