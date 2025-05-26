#pragma once

#include "Component/Texture.hpp"
#include "Component/Mesh.hpp"

#include "Geometry/QuadKey.hpp"

#include "Utility/PerlinNoise.hpp"

#include <unordered_map>

namespace System
{
	class AssetManager;
}
namespace Geometry
{
	class AABB2D;
}
namespace Component
{
	class Terrain
	{
		OpenGL::VAO VAO;
		OpenGL::Buffer vert_buffer;
		OpenGL::Buffer index_buffer;

		size_t chunk_vert_buff_stride() const;
		size_t chunk_index_buff_stride() const;
		void regenerate_mesh();
		void remove_verts(const Geometry::QuadKey& key);
		void add_verts(const Geometry::QuadKey& info);
		float compute_height(float p_x, float p_z, const siv::PerlinNoise& perlin);

		// Given a player_pos to center around, return the quadkeys which represent the leaf nodes of the quad tree centered around that pos.
		std::vector<Geometry::QuadKey> get_tree_leaf_nodes();
		Geometry::Depth_t required_depth(const Geometry::AABB2D& AABB);

	public:
		std::unordered_map<Geometry::QuadKey, size_t> node_mesh_info; // Mapping of Geometry::QuadKey to index into the buffer data for the node.
		std::vector<size_t> free_indices; // Free indices in the buffer data for the node.
		size_t end_index = 0; // End index of the last node added to the buffer data.

		// Tree params
		glm::vec2 root_center; // Center of the root node.
		glm::vec2 player_pos;
		uint8_t max_depth;     // max depth of the quad tree.
		uint8_t chunk_detail;  // Number of vertices per chunk side.
		float root_size;       // Size of the root node.
		float decay_rate;

		// Noise params
		constexpr static size_t Persistent_ID = 6;
		float m_scale_factor;
		float m_amplitude;
		float m_lacunarity;
		float m_persistence;
		int m_octaves;

		TextureRef m_grass_tex;
		TextureRef m_gravel_tex;
		TextureRef m_ground_tex;
		TextureRef m_rock_tex;
		TextureRef m_sand_tex;
		TextureRef m_snow_tex;

		unsigned int m_seed; // Seed used to generate m_mesh.

		Terrain(float amplitude) noexcept;
		Terrain& operator=(Terrain&& p_other) noexcept = default;
		Terrain(Terrain&& p_other)            noexcept = default;
		Terrain(const Terrain& p_other);                // = delete;
		Terrain& operator=(const Terrain& p_other);     // = delete;

		void update(const glm::vec3& p_player_pos, float view_distance);
		bool empty() const { return node_mesh_info.empty();}
		void draw_UI(System::AssetManager& p_asset_manager);
		OpenGL::VAO& get_VAO() { return VAO; }
	};
} // namespace Component