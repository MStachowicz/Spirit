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
	Textured 		= 0,
	Material 		= 1,
	UniformColour 	= 2,

	Count
};
// Allows iterating over enum class DrawStyle
static const std::array<std::string, util::toIndex(DrawStyle::Count)> drawStyles { "Textured", "Material", "Uniform Colour" };
static std::string convert(const DrawStyle& pDrawStyle)
{
	return drawStyles[util::toIndex(pDrawStyle)];
}

struct Material
{
    glm::vec3 ambient = glm::vec3(1.0f, 0.5f, 0.31f);
    glm::vec3 diffuse = glm::vec3(1.0f, 0.5f, 0.31f);
    glm::vec3 specular = glm::vec3(0.5f, 0.5f, 0.5f);
    float shininess = 32.0f;
};

// A request to execute a specific draw using a GraphicsAPI.
struct DrawCall
{
	MeshID 					 mMesh;
	std::optional<TextureID> mTexture	= std::nullopt;
	std::optional<Material>  mMaterial	= std::nullopt;
	std::optional<glm::vec3> mColour	= std::nullopt;

	DrawMode 	mDrawMode  	= DrawMode::Fill;
	DrawStyle	mDrawStyle  = DrawStyle::Textured;

	glm::vec3 	mPosition 	= glm::vec3(0.0f);
	glm::vec3 	mRotation	= glm::vec3(0.0f);
	glm::vec3 	mScale		= glm::vec3(1.0f);
};