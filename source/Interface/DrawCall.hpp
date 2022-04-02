#pragma once

#include "Utility.hpp"
#include "Types.hpp"

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
	UniformColour 	= 1,
	LightMap		= 2,

	Count
};
// Allows iterating over enum class DrawStyle
static const std::array<std::string, util::toIndex(DrawStyle::Count)> drawStyles { "Textured", "Uniform Colour", "Light Map" };
static std::string convert(const DrawStyle& pDrawStyle)
{
	return drawStyles[util::toIndex(pDrawStyle)];
}

// A request to execute a specific draw using a GraphicsAPI.
struct DrawCall
{
	MeshID 					 mMesh;
	DrawMode 	mDrawMode  	= DrawMode::Fill;
	DrawStyle	mDrawStyle  = DrawStyle::Textured;
	glm::vec3 	mPosition 	= glm::vec3(0.0f);
	glm::vec3 	mRotation	= glm::vec3(0.0f);
	glm::vec3 	mScale		= glm::vec3(1.0f);

	//DrawStyle::Textured
	std::optional<TextureID> mTexture1	= std::nullopt;
	std::optional<TextureID> mTexture2	= std::nullopt;
	std::optional<float> 	 mMixFactor = std::nullopt; // If mTexture1 and mTexture2 are set, allows setting the balance between the two textures.
	//DrawStyle::UniformColour
	std::optional<glm::vec3> mColour	= std::nullopt;
	//DrawStyle::LightMap
	std::optional<TextureID> mDiffuseTextureID	= std::nullopt;
	std::optional<TextureID> mSpecularTextureID	= std::nullopt;
	std::optional<float> 	 mShininess			= std::nullopt;

};