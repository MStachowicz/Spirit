#pragma once

// Component
#include "Texture.hpp"

// Geometry
#include "AABB.hpp"
#include "Triangle.hpp"

// OpenGL
#include "Types.hpp"

// GLM
#include "glm/vec4.hpp"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

// Utility
#include "ResourceManager.hpp"

// STD
#include <filesystem>
#include <vector>

struct aiMesh;
struct aiNode;
struct aiScene;

namespace Data
{
	class Vertex
	{
	public:
		glm::vec3 position = glm::vec3{0.f};
		glm::vec3 normal   = glm::vec3{0.f};
		glm::vec2 uv       = glm::vec2{0.f};
		glm::vec4 colour   = glm::vec4{1.f};
	};

	class NewMesh // TODO replace OpenGL::Mesh and Data::Mesh with this
	{
		OpenGL::VAO VAO   = {};
		OpenGL::VBO VBO   = {};
		GLsizei draw_size = 0;
		OpenGL::PrimitiveMode primitive_mode;

	public:
		void draw()
		{
			VAO.bind();
			OpenGL::draw_arrays(primitive_mode, 0, draw_size);
		}

		NewMesh(const std::vector<Vertex>& vertex_data, OpenGL::PrimitiveMode primitive_mode)
			: VAO{}
			, VBO{}
			, draw_size{(GLsizei)vertex_data.size()}
			, primitive_mode{primitive_mode}
		{
			VAO.bind();
			VBO.bind();

			OpenGL::buffer_data(OpenGL::BufferType::ArrayBuffer, vertex_data.size() * sizeof(Vertex), vertex_data.data(), OpenGL::BufferUsage::StaticDraw);

			OpenGL::vertex_attrib_pointer(0, 3, OpenGL::ShaderDataType::Float, false, sizeof(Vertex), (void*)offsetof(Vertex, position));
			OpenGL::vertex_attrib_pointer(1, 3, OpenGL::ShaderDataType::Float, false, sizeof(Vertex), (void*)offsetof(Vertex, normal));
			OpenGL::vertex_attrib_pointer(2, 4, OpenGL::ShaderDataType::Float, false, sizeof(Vertex), (void*)offsetof(Vertex, colour));
			OpenGL::vertex_attrib_pointer(3, 2, OpenGL::ShaderDataType::Float, false, sizeof(Vertex), (void*)offsetof(Vertex, uv));

			OpenGL::enable_vertex_attrib_array(0);
			OpenGL::enable_vertex_attrib_array(1);
			OpenGL::enable_vertex_attrib_array(2);
			OpenGL::enable_vertex_attrib_array(3);
		}
	};
}

namespace Data
{
    struct Material
    {
        // If the Mesh has a pre-defined texture associated it is set here.
        TextureRef mDiffuseTexture;
        TextureRef mSpecularMap;
        TextureRef mHeightMap;
        TextureRef mAmbientMap;
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

        Geometry::AABB mAABB; // Object space AABB encompassing all the mPositions of the mesh.
        std::vector<Geometry::Triangle> mTriangles; // Object space triangles of the mesh.

        Material mMaterial;
        OpenGL::Mesh mGLData; // The GPU representation of the data.

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


        // Recursively call pFunction on every Mesh in the mesh tree.
        // pFunction will only be applied on meshes on this level and deeper.
        template<typename Func>
        void forEachMesh(const Func&& pFunction) const
        {
            mCompositeMesh.forEachMesh(std::forward<const Func>(pFunction));
        }

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