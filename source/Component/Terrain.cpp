#include "Terrain.hpp"

#include "System/AssetManager.hpp"
#include "Utility/MeshBuilder.hpp"
#include "Utility/PerlinNoise.hpp"
#include "Utility/Stopwatch.hpp"

#include "Geometry/AABB.hpp"

#include "imgui.h"

namespace Component
{
	using VertexType = Data::Vertex;
	constexpr size_t size_of_vertex = sizeof(VertexType);

	size_t Terrain::chunk_vert_buff_stride() const
	{
		return (chunk_detail + 1) * (chunk_detail + 1) * sizeof(VertexType); // Verts per chunk * size of each vert
	}
	size_t Terrain::chunk_index_buff_stride() const
	{
		return chunk_detail * chunk_detail * 6 * sizeof(unsigned int); // Indices per chunk * size of each index
	}
	void Terrain::regenerate_mesh()
	{
		vert_buffer.clear(0, vert_buffer.used_capacity());
		index_buffer.clear(0, index_buffer.used_capacity());

		for (auto& node : quad_tree)
		{
			if (node.leaf()) // Only add verts to leaf nodes
				add_verts(node);
		}
	}
	void Terrain::clear_node_data(size_t index)
	{
		const size_t vert_buff_stride  = chunk_vert_buff_stride();
		const size_t index_buff_stride = chunk_index_buff_stride();
		vert_buffer.clear(index * vert_buff_stride, vert_buff_stride);
		index_buffer.clear(index  * index_buff_stride, index_buff_stride);
		LOG("[TERRAIN] Clearing node {} data. Verts: {} ({}B), Indices: {} ({}B)", index, vert_buff_stride / sizeof(VertexType), vert_buff_stride, index_buff_stride / sizeof(unsigned int), index_buff_stride);
	}
	void Terrain::add_verts(const QuadTree::Node& node)
	{
		LOG("[TERRAIN] Generating terrain for node {} with bounds: {:.0f},{:.0f} -> {:.0f},{:.0f}", quad_tree.node_index(node), node.bounds.min.x, node.bounds.min.y, node.bounds.max.x, node.bounds.max.y);
		ASSERT(chunk_detail > 0, "Chunk detail must be greater than 0");

		std::vector<VertexType> new_verts;
		new_verts.reserve(chunk_vert_buff_stride() / size_of_vertex);
		std::vector<unsigned int> new_indices;
		new_indices.reserve(chunk_index_buff_stride() / sizeof(unsigned int));

		const size_t node_index = quad_tree.node_index(node);
		const auto& bounds      = node.bounds;
		//const auto perlin       = siv::PerlinNoise{m_seed};
		const float chunk_step  = (bounds.max.x - bounds.min.x) / chunk_detail;

		for (uint16_t z = 0; z <= chunk_detail; ++z)
		{
			for (uint16_t x = 0; x <= chunk_detail; ++x)
			{
				VertexType vert;
				float pos_x   = bounds.min.x + x * chunk_step;
				float pos_z   = bounds.min.y + z * chunk_step;
				float pos_y   = 0.f;//compute_height(pos_x, pos_z, perlin);
				vert.position = {pos_x, pos_y, pos_z};
				vert.normal   = {0.f, 0.f, 0.f}; // Calculate normals later
				new_verts.push_back(vert);
			}
		}

		// Generate indices for each quad (two triangles per quad)
		for (int z = 0; z < chunk_detail; ++z)
		{
			for (int x = 0; x < chunk_detail; ++x)
			{
				// Get the indices for the four corners of the current quad
				unsigned int top_left     = z * (chunk_detail + 1) + x;
				unsigned int top_right    = top_left + 1;
				unsigned int bottom_left  = (z + 1) * (chunk_detail + 1) + x;
				unsigned int bottom_right = bottom_left + 1;

				// Add the indices for the two triangles that make up the quad
				// Triangle 1: top left, bottom left, top right
				new_indices.push_back(top_left);
				new_indices.push_back(bottom_left);
				new_indices.push_back(top_right);
				// Triangle 2: top right, bottom left, bottom right
				new_indices.push_back(top_right);
				new_indices.push_back(bottom_left);
				new_indices.push_back(bottom_right);

				// Calculate normals for the first triangle then accumulate the normal to each of the triangle's vertices
				glm::vec3 v0     = new_verts[top_left].position;
				glm::vec3 v1     = new_verts[bottom_left].position;
				glm::vec3 v2     = new_verts[top_right].position;
				glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
				new_verts[top_left].normal    += normal;
				new_verts[bottom_left].normal += normal;
				new_verts[top_right].normal   += normal;

				// Calculate normals for the second triangle
				// Accumulate the normal to each of the triangle's vertices
				v0     = new_verts[top_right].position;
				v1     = new_verts[bottom_left].position;
				v2     = new_verts[bottom_right].position;
				normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
				new_verts[top_right].normal    += normal;
				new_verts[bottom_left].normal  += normal;
				new_verts[bottom_right].normal += normal;

				// Calculate UV coordinates based on vertex position within the terrain grid.
				// UV coordinates range from 0.0 to size_x or size_z
				float u = static_cast<float>(x);
				float v = static_cast<float>(z);
				new_verts[top_left].uv     = {u, v + 1.0f};
				new_verts[bottom_left].uv  = {u, v};
				new_verts[top_right].uv    = {u + 1.0f, v + 1.0f};
				new_verts[bottom_right].uv = {u + 1.0f, v};
			}
		}

		// Normalize the vertex normals
		for (auto& vert : new_verts)
			vert.normal = glm::normalize(vert.normal);

		{ // Push the new verts to their offset in the buffer
			const size_t chunk_stride = chunk_vert_buff_stride();
			const size_t chunk_offset = node_index * chunk_stride;

			if (vert_buffer.capacity() < chunk_offset + chunk_stride)
			{
				const size_t new_capacity = Utility::next_power_of_2(vert_buffer.capacity() + chunk_stride);
				auto current_cap          = Utility::format_number(vert_buffer.used_capacity());
				auto new_cap_formatted    = Utility::format_number(new_capacity);
 				LOG("[TERRAIN] Resizing terrain buffer from {}B to {}B", current_cap, new_cap_formatted);
				vert_buffer.reserve(new_capacity);
			}
			vert_buffer.set_data(new_verts, chunk_offset);
		}

		{ // Push the new indices to the offset in the buffer
			const size_t chunk_stride = chunk_index_buff_stride();
			const size_t chunk_offset = node_index * chunk_stride;

			if (index_buffer.capacity() < chunk_offset + chunk_stride)
			{
				const size_t new_capacity = Utility::next_power_of_2(index_buffer.capacity() + chunk_stride);
				auto current_cap          = Utility::format_number(index_buffer.used_capacity());
				auto new_cap_formatted    = Utility::format_number(new_capacity);
				LOG("[TERRAIN] Resizing index buffer from {}B to {}B", current_cap, new_cap_formatted);
				index_buffer.reserve(new_capacity);
			}
			{// Offset the indices calculated by the number of unique vertices in the buffer
				const unsigned int offset = (unsigned int)((chunk_detail + 1) * (chunk_detail + 1) * node_index);
				for (auto& index : new_indices)
					index += offset;
			}
			index_buffer.set_data(new_indices, chunk_offset);
		}

		VAO.attach_buffer(vert_buffer, 0, 0, sizeof(VertexType), (GLsizei)(vert_buffer.used_capacity() / sizeof(VertexType)));
		VAO.attach_element_buffer(index_buffer, (GLsizei)(index_buffer.used_capacity() / sizeof(unsigned int)));
	}

	Terrain::Terrain(float amplitude) noexcept
		: VAO{}
		, vert_buffer{{OpenGL::BufferStorageFlag::DynamicStorageBit}}
		, index_buffer{{OpenGL::BufferStorageFlag::DynamicStorageBit}}
		, quad_tree{}
		, max_lod{0}
		, chunk_detail{2}
		, m_scale_factor{0.03f}
		, m_amplitude{amplitude}
		, m_lacunarity{2.f}
		, m_persistence{0.5f}
		, m_octaves{4}
		, m_grass_tex{}
		, m_gravel_tex{}
		, m_ground_tex{}
		, m_rock_tex{}
		, m_sand_tex{}
		, m_snow_tex{}
		, m_seed{Utility::get_random_number<unsigned int>()}
	{
		constexpr GLint vertex_buffer_binding_point = 0;
		VAO.set_vertex_attrib_pointers(OpenGL::PrimitiveMode::Triangles, {
			{0, 3, OpenGL::BufferDataType::Float, offsetof(VertexType, position), vertex_buffer_binding_point, false},
			{1, 3, OpenGL::BufferDataType::Float, offsetof(VertexType, normal),   vertex_buffer_binding_point, false},
			{3, 2, OpenGL::BufferDataType::Float, offsetof(VertexType, uv),       vertex_buffer_binding_point, false},
			{2, 4, OpenGL::BufferDataType::Float, offsetof(VertexType, colour),   vertex_buffer_binding_point, false}
		});

		quad_tree.add_root_node(Geometry::AABB2D({0, 0}, {100, 100}), BufferHandle{});
		add_verts(quad_tree.root_node());
	}
	Terrain::Terrain(const Terrain& p_other)
		: VAO{}
		, vert_buffer{{OpenGL::BufferStorageFlag::DynamicStorageBit}}
		, index_buffer{{OpenGL::BufferStorageFlag::DynamicStorageBit}}
		, quad_tree{}
		, max_lod{}
		, chunk_detail{100}
		, m_scale_factor{}
		, m_amplitude{}
		, m_lacunarity{}
		, m_persistence{}
		, m_octaves{}
		, m_grass_tex{}
		, m_gravel_tex{}
		, m_ground_tex{}
		, m_rock_tex{}
		, m_sand_tex{}
		, m_snow_tex{}
		, m_seed{Utility::get_random_number<unsigned int>()}
	{(void)p_other;
		ASSERT_FAIL("ECS Storage does not support non-copyable components. Until support is added, this component must be copyable.");
	}
	Terrain& Terrain::operator=(const Terrain& p_other)
	{(void)p_other;
		ASSERT_FAIL("ECS Storage does not support non-copyable components. Until support is added, this component must be copyable.");
	}

	float Terrain::compute_height(float p_x, float p_z, const siv::PerlinNoise& perlin)
	{
		float accumulate       = 0.f;
		float octave_frequency = 1.f;
		float octave_amplitude = 1.f;
		float max_value        = 0.f;

		for (int i = 0; i < m_octaves; i++)
		{
			accumulate       += static_cast<float>(perlin.noise2D(p_x * m_scale_factor * octave_frequency, p_z * m_scale_factor * octave_frequency)) * octave_amplitude;
			max_value        += octave_amplitude;
			octave_amplitude *= m_persistence;
			octave_frequency *= m_lacunarity;
		}

		return accumulate / max_value * m_amplitude;
	}

	void Terrain::update(const glm::vec3& player_pos, float view_distance)
	{(void)player_pos; (void)view_distance;
		ImGui::Begin("Terrain");
		ImGui::Text("Chunks", quad_tree.size());
		ImGui::Text("Depth", quad_tree.depth());
		ImGui::Text("Max LOD", max_lod);
		ImGui::Text("Chunk detail", (int)chunk_detail);
		ImGui::Text("Vert count ", vert_buffer.used_capacity() / sizeof(VertexType));
		ImGui::Text("Index count", index_buffer.used_capacity() / sizeof(unsigned int));
		ImGui::Text("Vert buffer size", vert_buffer.used_capacity());
		ImGui::Text("Index buffer size", index_buffer.used_capacity());
		ImGui::Text("Draw count", VAO.draw_count());

		int im_chunk_detail = chunk_detail;
		if (ImGui::SliderInt("Chunk detail", &im_chunk_detail, 1, std::numeric_limits<uint8_t>::max()))
		{
			chunk_detail = (uint8_t)im_chunk_detail;
			regenerate_mesh();
		}

		if (ImGui::Button("Subdivide"))
		{
			quad_tree.depth_first_traversal([&](auto& node)
			{
				if (node.leaf())
				{
					LOG("[TERRAIN] Subdividing node {} with bounds: {:.0f},{:.0f} -> {:.0f},{:.0f}", quad_tree.node_index(node), node.bounds.min.x, node.bounds.min.y, node.bounds.max.x, node.bounds.max.y);
					size_t node_index = quad_tree.node_index(node); // Get the index of the node before add_node may invalidate the reference
					quad_tree.subdivide(node,
						BufferHandle{},
						BufferHandle{},
						BufferHandle{},
						BufferHandle{});

					auto& node_after = quad_tree[node_index];
					quad_tree.for_each_child(node_after, [&](const auto& child) { add_verts(child); });
					clear_node_data(node_index);
					return true;
				}
				else
					return false;
			});
		}
		if (ImGui::Button("Merge"))
		{
			const size_t max_depth = quad_tree.depth();

			if (max_depth > 0)
			{
				const size_t target_depth = max_depth - 1;

				quad_tree.depth_first_traversal([&](auto& node)
				{
					if (node.depth == target_depth && !node.leaf())
					{
						LOG("[TERRAIN] Merging node {} with bounds: {:.0f},{:.0f} -> {:.0f},{:.0f}", quad_tree.node_index(node), node.bounds.min.x, node.bounds.min.y, node.bounds.max.x, node.bounds.max.y);
						auto child_indices = *node.children_indices;
						quad_tree.merge(node);

						for (auto& index : child_indices)
							clear_node_data(index);

						// Reconstruct the verts and indices for the parent node
						add_verts(node);

						return true;
					}
					else
						return false;
				});
			}
		}

		ImGui::End();
	}

	void Terrain::draw_UI(System::AssetManager& p_asset_manager)
	{
		if (ImGui::TreeNode("Terrain"))
		{
			ImGui::SeparatorText("Textures");
			p_asset_manager.draw_texture_selector("Grass texture", m_grass_tex);
			p_asset_manager.draw_texture_selector("Gravel texture", m_gravel_tex);
			p_asset_manager.draw_texture_selector("Rock texture", m_rock_tex);
			p_asset_manager.draw_texture_selector("Ground texture", m_ground_tex);
			p_asset_manager.draw_texture_selector("Sand texture", m_sand_tex);
			p_asset_manager.draw_texture_selector("Snow texture", m_snow_tex);

			ImGui::SeparatorText("Generation settings");
			static bool m_regen_on_changes = true;
			bool changed = false;
			changed |= ImGui::Slider("Scale factor", m_scale_factor, 0.01f , 0.15f);
			changed |= ImGui::Slider("Amplitude", m_amplitude, 0.01f, 100.f);
			changed |= ImGui::Slider("Lacunarity", m_lacunarity, 0.01f, 4.f);
			changed |= ImGui::Slider("Persistence", m_persistence, 0.01f, 1.f);
			changed |= ImGui::Slider("Octaves", m_octaves, 1, 10);
			changed |= ImGui::InputScalar("Seed", ImGuiDataType_U32, &m_seed);
			ImGui::SameLine();
			if (ImGui::Button("Rand"))
			{
				changed = true;
				m_seed  = Utility::get_random_number<unsigned int>();
			}

			static auto most_recent_time_taken_s = std::optional<float>{};
			if (ImGui::Button("Re-generate terrain") || (changed && m_regen_on_changes))
			{
				Utility::Stopwatch stopwatch;
				regenerate_mesh();
				most_recent_time_taken_s = stopwatch.getTime<std::ratio<1, 1>, float>();
			}
			if (most_recent_time_taken_s)
			{
				ImGui::SameLine();
				auto formatted_time = Utility::format_number(*most_recent_time_taken_s, 1);
				ImGui::Text_Manual("%ss", formatted_time.c_str());
			}

			ImGui::SameLine();
			ImGui::Checkbox("Regen on changes", &m_regen_on_changes);


			ImGui::TreePop();
		}
	}
} // namespace Component