#pragma once

#include "Mesh.hpp"

#include "glm/vec3.hpp"	// vec3, bvec3, dvec3, ivec3 and uvec3

#include "unordered_map"
#include "functional"

namespace std
{
    namespace filesystem
    {
        class path;
    }
}

class TextureManager;

class MeshManager
{
public:
    MeshManager(TextureManager& pTextureManager)
    : mTextureManager(pTextureManager)
    {
        buildMeshes();
    }

    inline void ForEach(const std::function<void(const Mesh&)>& pFunction) const
    {
        for (const auto& mesh : mMeshes)
            pFunction(mesh.second);
    }

	MeshID getMeshID(const std::string& pMeshName);

    // Loads model data using ASSIMP from pFilePath.
    MeshID loadModel(const std::filesystem::path& pFilePath);

private:
    std::unordered_map<MeshID, Mesh> mMeshes;
    std::unordered_map<std::string, MeshID> mMeshNames;
    TextureManager& mTextureManager; // Owned by Renderer.

    void addMesh(const Mesh& pMesh)
    {
        mMeshes.emplace(std::make_pair(pMesh.mID, pMesh));
        mMeshNames.emplace(std::make_pair(pMesh.mName, pMesh.mID));
    }

    void buildMeshes(); // Populates mMeshes with some commonly used shapes
};