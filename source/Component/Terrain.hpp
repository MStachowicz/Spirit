#pragma once

#include "Component/Texture.hpp"
#include "Component/Mesh.hpp"

#include "Geometry/QuadTree.hpp"

#include "Utility/PerlinNoise.hpp"

namespace System
{
	class AssetManager;
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
		void clear_node_data(size_t index);
		float compute_height(float p_x, float p_z, const siv::PerlinNoise& perlin);
	public:
		struct BufferHandle
		{
			size_t index;
			size_t size;
			OpenGL::Buffer* vert_buffer;
			OpenGL::Buffer* index_buffer;

			BufferHandle()
				: index{}
				, size{}
				, vert_buffer{}
				, index_buffer{}
			{}
			~BufferHandle()
			{}

			void on_subdivide()
			{

			}
		};
		using QuadTree = Geometry::QuadTree<BufferHandle>;
		QuadTree quad_tree;
		uint8_t max_lod; // max depth of the quad tree.
		uint8_t chunk_detail; // Number of vertices per chunk side.

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

		void add_verts(const QuadTree::Node& node);
		void update(const glm::vec3& p_player_pos, float view_distance);
		bool empty() const { return quad_tree.empty();}
		void draw_UI(System::AssetManager& p_asset_manager);
		OpenGL::VAO& get_VAO() { return VAO; }
	};
} // namespace Component