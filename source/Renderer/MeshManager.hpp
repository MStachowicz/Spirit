#pragma once

#include "Mesh.hpp"

#include "glm/vec3.hpp" // vec3, bvec3, dvec3, ivec3 and uvec3

#include <functional>
#include <unordered_map>

namespace std
{
    namespace filesystem
    {
        class path;
    }
} // namespace std

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;

namespace Manager
{
    class TextureManager;

    class MeshManager
    {
    public:
        MeshManager(TextureManager& pTextureManager)
            : mTextureManager(pTextureManager)
        {
            buildMeshes();
        }

        inline void ForEach(const std::function<void(const Data::Mesh&)>& pFunction) const
        {
            for (const auto& mesh : mMeshes)
                pFunction(mesh);
        }

        MeshID getMeshID(const std::string& pMeshName);

        // Loads model data using ASSIMP from pFilePath.
        MeshID loadModel(const std::filesystem::path& pFilePath);

    private:
        std::vector<Data::Mesh> mMeshes;
        std::unordered_map<std::string, size_t> mMeshNames;

        Manager::TextureManager& mTextureManager; // Owned by Renderer.

        // Set the mID of the mesh and its children recursively.
        void setIDRecursively(Data::Mesh& pMesh, const bool& pRootMesh);

        void addMesh(Data::Mesh& pMesh);
        void buildMeshes();
        bool isMeshValid(const Data::Mesh& pMesh);

        // Recursively travel all the aiNodes and extract the per-vertex data into a Zephyr mesh object.
        void processNode(Data::Mesh& pParentMesh, aiNode* pNode, const aiScene* pScene);
        // Load assimp mesh data into a Zephyr Mesh.
        void processData(Data::Mesh& pMesh, const aiMesh* pAssimpMesh, const aiScene* pAssimpScene);
        // Returns all the textures for this material and purpose.
        void processTextures(Data::Mesh& pMesh, aiMaterial* pMaterial, const Data::Texture::Purpose& pPurpose);
    };
} // namespace Manager