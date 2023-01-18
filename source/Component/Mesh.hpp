#pragma once

// Component
#include "Texture.hpp"

// Geometry
#include "AABB.hpp"

// OpenGL
#include "Types.hpp"

// GLM
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

// Utility
#include "ResourceManager.hpp"

// STD
#include <filesystem>
#include <optional>
#include <vector>

struct aiMesh;
struct aiNode;
struct aiScene;

namespace Data
{
    struct Material
    {
        // If the Mesh has a pre-defined texture associated it is set here.
        std::optional<TextureRef> mDiffuseTexture;
        std::optional<TextureRef> mSpecularMap;
        std::optional<TextureRef> mHeightMap;
        std::optional<TextureRef> mAmbientMap;
    };

    // An implicit mesh, a collection of vertices defining a 3D triangulated Mesh.
    // The mesh stores additional data such as the rendering backend, AABB in its object space and the Material data if any.
    class Mesh
    {
    public:
        std::vector<glm::vec3> mPositions;
        std::vector<glm::vec3> mNormals;
        std::vector<glm::vec2> mTextureCoordinates;
        std::vector<int> mIndices;

        Geometry::AABB mAABB;

        Material mMaterial;
        OpenGL::Mesh mGLData;

        Mesh() noexcept = default;
        Mesh(aiMesh& pAssimpMesh) noexcept;
        Mesh(aiMesh& pAssimpMesh, const aiScene& pAssimpScene, TextureManager& pTextureManager) noexcept;
    };





    // A node of a Mesh tree, the CompositeMesh can own multiple Meshes and any number of child CompositeMeshes
    class CompositeMesh
    {
    public:
        std::vector<Mesh> mMeshes;
        std::vector<CompositeMesh> mChildMeshes;
        Geometry::AABB mAABB;

        CompositeMesh() noexcept
            : mMeshes{}
            , mChildMeshes{}
            , mAABB{}
        {}
        // Recursively construct many CompositeMeshes by navigating the assimp nodes.
        CompositeMesh(aiNode& pAssimpNode, const aiScene& pAssimpScene, TextureManager& pTextureManager) noexcept;

        // Recursively call pFunction on every Mesh in the mesh tree.
        // pFunction will only be applied on meshes on this level and deeper.
        template<typename Func>
        void forEachMesh(const Func&& pFunction)
        {
            for (auto& mesh : mMeshes)
                pFunction(mesh);
            for (auto& mesh : mChildMeshes)
                mesh.forEachMesh(std::forward<const Func>(pFunction));
        }
        // Recursively call pFunction on every Mesh in the mesh tree.
        // pFunction will only be applied on meshes on this level and deeper.
        template<typename Func>
        void forEachMesh(const Func&& pFunction) const
        {
            for (const auto& mesh : mMeshes)
                pFunction(mesh);
            for (const auto& mesh : mChildMeshes)
                mesh.forEachMesh(std::forward<const Func>(pFunction));
        }
    };
    // A Model is a tree structure of Mesh objects.
    // The Model acts as the root node owning the first CompositeMesh.
    class Model
    {
    public:
        Model(const std::filesystem::path& pFilePath, TextureManager& pTextureManager) noexcept;

        std::filesystem::path mFilePath;
        CompositeMesh mCompositeMesh; // The root node of a mesh tree.
    };


} // namespace Data

// Manages the lifetime of reference counted Data::Model objects.
using ModelManager = Utility::ResourceManager<Data::Model>;
// A resource counted wrapper for a pointer to a Data::Model object.
using ModelRef     = Utility::ResourceRef<Data::Model>;

namespace Component
{
    class Mesh
    {
    public:
        Mesh(const ModelRef& pModel);
        ~Mesh() = default;

        ModelRef mModel;
    };
}