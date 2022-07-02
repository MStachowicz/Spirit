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

struct DrawCallInstanceHash
{
	std::size_t operator()(const DrawCall& drawCall) const
	{
		std::size_t seed = 0;
		util::HashCombine(seed, drawCall.mMesh.mID);
		util::HashCombine(seed, drawCall.mMesh.mDrawMode);
		util::HashCombine(seed, drawCall.mMesh.mDrawStyle);

		// Per DrawStyle values
		util::HashCombine(seed, drawCall.mMesh.mTexture1);
		util::HashCombine(seed, drawCall.mMesh.mTexture2);
		util::HashCombine(seed, drawCall.mMesh.mMixFactor);
		util::HashCombine(seed, drawCall.mMesh.mColour);
		util::HashCombine(seed, drawCall.mMesh.mDiffuseTextureID);
		util::HashCombine(seed, drawCall.mMesh.mSpecularTextureID);
		util::HashCombine(seed, drawCall.mMesh.mShininess);
		util::HashCombine(seed, drawCall.mMesh.mTextureRepeatFactor);

		return seed;
	}
};

struct DrawCallInstanceEqual
{
	bool operator()(const DrawCall& pDrawCall1, const DrawCall& pDrawCall2) const
	{
		return pDrawCall1.mMesh.mID              == pDrawCall2.mMesh.mID
		&& pDrawCall1.mMesh.mDrawMode            == pDrawCall2.mMesh.mDrawMode
		&& pDrawCall1.mMesh.mDrawStyle           == pDrawCall2.mMesh.mDrawStyle

		// Per DrawStyle values
		&& pDrawCall1.mMesh.mTexture1            == pDrawCall2.mMesh.mTexture1
		&& pDrawCall1.mMesh.mTexture2            == pDrawCall2.mMesh.mTexture2
		&& pDrawCall1.mMesh.mMixFactor           == pDrawCall2.mMesh.mMixFactor
		&& pDrawCall1.mMesh.mColour              == pDrawCall2.mMesh.mColour
		&& pDrawCall1.mMesh.mDiffuseTextureID    == pDrawCall2.mMesh.mDiffuseTextureID
		&& pDrawCall1.mMesh.mSpecularTextureID   == pDrawCall2.mMesh.mSpecularTextureID
		&& pDrawCall1.mMesh.mShininess           == pDrawCall2.mMesh.mShininess
		&& pDrawCall1.mMesh.mTextureRepeatFactor == pDrawCall2.mMesh.mTextureRepeatFactor;
	}
};