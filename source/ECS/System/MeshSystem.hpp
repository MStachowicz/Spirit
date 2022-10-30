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

namespace System
{
    class TextureSystem;

    // MeshSystem is a container of the raw Mesh data.
    class MeshSystem
    {
    private:
        std::vector<Component::Mesh> mMeshes;
        std::unordered_map<std::string, size_t> mMeshNames;
        System::TextureSystem& mTextureSystem;

    public:
        MeshSystem(TextureSystem& pTextureSystem)
            : mTextureSystem(pTextureSystem)
        {
            buildMeshes();
        }

        inline void ForEach(const std::function<void(const Component::Mesh&)>& pFunction) const
        {
            for (const auto& mesh : mMeshes)
                pFunction(mesh);
        }

        Component::MeshID getMeshID(const std::string& pMeshName) const;

        // Loads model data using ASSIMP from pFilePath.
        Component::MeshID loadModel(const std::filesystem::path& pFilePath);

    private:
        // Set the mID of the mesh and its children recursively.
        void setIDRecursively(Component::Mesh& pMesh, const bool& pRootMesh);

        void addMesh(Component::Mesh& pMesh);
        void buildMeshes();
        bool isMeshValid(const Component::Mesh& pMesh);

        // Recursively travel all the aiNodes and extract the per-vertex data into a Zephyr mesh object.
        void processNode(Component::Mesh& pParentMesh, aiNode* pNode, const aiScene* pScene);
        // Load assimp mesh data into a Zephyr Mesh.
        void processData(Component::Mesh& pMesh, const aiMesh* pAssimpMesh, const aiScene* pAssimpScene);
        // Returns all the textures for this material and purpose.
        void processTextures(Component::Mesh& pMesh, aiMaterial* pMaterial, const Component::Texture::Purpose& pPurpose);
    };
} // namespace System