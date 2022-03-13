#pragma once

#include "Mesh.hpp"
#include "Texture.hpp"

#include "Utility.hpp"

#include "glm/vec3.hpp"	// vec3, bvec3, dvec3, ivec3 and uvec3

#include "optional"
#include "array"

enum class DrawMode
{
	Fill,
	Wireframe,

	Count
};
// Allows iterating over enum class DrawMode
static const std::array<std::string, util::toIndex(DrawMode::Count)> drawModes { "Fill", "Wireframe" };
static std::string convert(const DrawMode& pDrawMode)
{
	return drawModes[util::toIndex(pDrawMode)];
}

enum class DrawStyle : size_t
{
	Default 		= 0,
	Textured 		= 1,
	Material 		= 2,
	UniformColour 	= 3,

	Count
};
// Allows iterating over enum class DrawStyle
static const std::array<std::string, util::toIndex(DrawStyle::Count)> drawStyles { "Default", "Textured", "Material", "Uniform Colour" };
static std::string convert(const DrawStyle& pDrawStyle)
{
	return drawStyles[util::toIndex(pDrawStyle)];
}

struct Material
{
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
};

// A request to execute a specific draw using a GraphicsAPI.
struct DrawCall
{
	MeshID 					 mMesh;
	std::optional<TextureID> mTexture;
	std::optional<Material>  mMaterial;
	std::optional<glm::vec3> mColour;

	DrawMode 	mDrawMode  	= DrawMode::Fill;
	DrawStyle	mDrawStyle  = DrawStyle::Default;

	glm::vec3 	mPosition 	= glm::vec3(0.0f);
	glm::vec3 	mRotation	= glm::vec3(0.0f);
	glm::vec3 	mScale		= glm::vec3(1.0f);
};