#include "Terrain.hpp"

#include "System/AssetManager.hpp"
#include "Utility/MeshBuilder.hpp"
#include "Utility/Performance.hpp"
#include "Utility/Stopwatch.hpp"
#include "Utility/Utility.hpp"

#include "OpenGL/DebugRenderer.hpp"

#include "imgui.h"

#include <bitset>

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
		node_mesh_info.clear();
		free_indices.clear();
		end_index = 0;
		LOG("[TERRAIN] Clearing all mesh data.");

		auto leaf_nodes = get_tree_leaf_nodes();
		for (const auto& node : leaf_nodes)
			add_verts(node);
	}

	Geometry::Depth_t Terrain::required_depth(const Geometry::AABB2D& bounds)
	{
		float dist       = bounds.distance(player_pos);
		float normalized = std::exp(-decay_rate * dist);
		return static_cast<Geometry::Depth_t>(std::round(normalized * max_depth));
	}
	std::vector<Geometry::QuadKey> Terrain::get_tree_leaf_nodes()
	{
		PERF(GetTreeLeafNodes);
		std::vector<Geometry::QuadKey> out_keys;

		if (root_bounds)
		{
			out_keys.reserve(static_cast<size_t>(std::pow(4, max_depth)));
			Geometry::generate_leaf_nodes(root_bounds->center, root_bounds->half_size, 0, 0, max_depth, out_keys, [this](const Geometry::AABB2D& bounds) { return this->required_depth(bounds); });
		}
		return out_keys;
	}

	void Terrain::remove_verts(MeshIter& to_remove)
	{
		//LOG("[TERRAIN] Removing node {} mesh data with bounds {}, {} -> {}, {}", key.to_string(), key.get_bounds(root_bounds->half_size, root_bounds->center).min.x, key.get_bounds(root_bounds->half_size, root_bounds->center).min.y, key.get_bounds(root_bounds->half_size, root_bounds->center).max.x, key.get_bounds(root_bounds->half_size, root_bounds->center).max.y);
		size_t data_index = to_remove->second;
		to_remove = node_mesh_info.erase(to_remove);

		if (data_index == end_index - 1)
			end_index--;
		else
			free_indices.push_back(data_index);

		size_t vert_buff_stride  = chunk_vert_buff_stride();
		size_t index_buff_stride = chunk_index_buff_stride();
		vert_buffer.clear(data_index * vert_buff_stride, vert_buff_stride);
		index_buffer.clear(data_index * index_buff_stride, index_buff_stride);

		// LOG("[TERRAIN] Clearing node {} vert data with bounds {}, {} -> {}, {}. Buffer section cleared from {}B to {}B", data_index, bounds.min.x, bounds.min.y, bounds.max.x, bounds.max.y, data_index * vert_buff_stride, data_index * vert_buff_stride + vert_buff_stride);
	}
	void Terrain::add_verts(const Geometry::QuadKey& key, std::optional<size_t> buffer_index_overwrite)
	{
		PERF(AddVerts);
		//LOG("[TERRAIN] Adding node {} mesh data with bounds {}, {} -> {}, {}", key.to_string(), key.get_bounds(root_bounds->half_size, root_bounds->center).min.x, key.get_bounds(root_bounds->half_size, root_bounds->center).min.y, key.get_bounds(root_bounds->half_size, root_bounds->center).max.x, key.get_bounds(root_bounds->half_size, root_bounds->center).max.y);

		ASSERT(chunk_detail > 0, "Chunk detail must be greater than 0");
		ASSERT(node_mesh_info.find(key) == node_mesh_info.end(), "Quadkey already exists in the mesh indices map.");
		ASSERT(root_bounds.has_value(), "Root bounds must be set before adding verts. Suggests a missing early out.");

		size_t data_index = [&]() -> size_t
		{
			if (buffer_index_overwrite.has_value())
				return buffer_index_overwrite.value();
			else if (free_indices.empty())
				return end_index++;
			else
			{
				size_t index = free_indices.back();
				free_indices.pop_back();
				return index;
			}
		}();
		node_mesh_info[key]     = data_index;
		auto bounds             = key.get_bounds(root_bounds->half_size, root_bounds->center);
		const float chunk_step  = (bounds.max.x - bounds.min.x) / chunk_detail;

		std::vector<VertexType> new_verts;
		{PERF(GenerateVerts);
			new_verts.reserve((chunk_detail + 1) * (chunk_detail + 1));
			for (uint16_t z = 0; z <= chunk_detail; ++z)
			{
				for (uint16_t x = 0; x <= chunk_detail; ++x)
				{
					VertexType vert;
					float pos_x   = bounds.min.x + x * chunk_step;
					float pos_z   = bounds.min.y + z * chunk_step;

					if (gen_normals_analytically)
					{
						auto result = Utility::Perlin::GetWithNormal(pos_x, pos_z, noise_params);
						vert.normal   = result.normal;
						vert.position = {pos_x, result.height, pos_z};
					}
					else
					{
						float pos_y   = static_cast<float>(Utility::Perlin::Get(pos_x, pos_z, noise_params));
						vert.position = {pos_x, pos_y, pos_z};
						vert.normal   = {0.f, 0.f, 0.f}; // Calculate normals later
					}
					new_verts.push_back(vert);
				}
			}
		}

		std::vector<unsigned int> new_indices;
		{
			PERF(GenerateIndices);
			new_indices.reserve(chunk_detail * chunk_detail * 6); // 6 indices per quad (2 triangles)
			// Generate indices for each quad (two triangles per quad)
			for (int z = 0; z < chunk_detail; ++z)
			{
				for (int x = 0; x < chunk_detail; ++x)
				{
					// Get the indices for the four corners of the current_node quad
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

					if (!gen_normals_analytically)
					{
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
					}

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
		}

		// Normalize the vertex normals
		for (auto& vert : new_verts)
			vert.normal = glm::normalize(vert.normal);

		{ // Push the new verts to their offset in the buffer
			PERF(SetVertBuffer);
			const size_t chunk_stride = chunk_vert_buff_stride();
			const size_t chunk_offset = data_index * chunk_stride;

			if (vert_buffer.capacity() < chunk_offset + chunk_stride)
			{
				PERF(ResizeVertBuffer);
				const size_t new_capacity = Utility::next_power_of_2(vert_buffer.capacity() + chunk_stride);
				auto current_cap          = Utility::format_number(vert_buffer.used_capacity());
				auto new_cap_formatted    = Utility::format_number(new_capacity);
 				LOG("[TERRAIN] Resizing terrain buffer from {}B to {}B", current_cap, new_cap_formatted);
				vert_buffer.reserve(new_capacity);
			}
			vert_buffer.set_data(new_verts, chunk_offset);
		}

		{ // Push the new indices to the offset in the buffer
			PERF(SetIndexBuffer);
			const size_t chunk_stride = chunk_index_buff_stride();
			const size_t chunk_offset = data_index * chunk_stride;

			if (index_buffer.capacity() < chunk_offset + chunk_stride)
			{
				PERF(ResizeIndexBuffer);
				const size_t new_capacity = Utility::next_power_of_2(index_buffer.capacity() + chunk_stride);
				auto current_cap          = Utility::format_number(index_buffer.used_capacity());
				auto new_cap_formatted    = Utility::format_number(new_capacity);
				LOG("[TERRAIN] Resizing index buffer from {}B to {}B", current_cap, new_cap_formatted);
				index_buffer.reserve(new_capacity);
			}
			{// Offset the indices calculated by the number of unique vertices in the buffer
				const unsigned int offset = (unsigned int)((chunk_detail + 1) * (chunk_detail + 1) * data_index);
				for (auto& index : new_indices)
					index += offset;
			}
			index_buffer.set_data(new_indices, chunk_offset);
		}

		VAO.attach_buffer(vert_buffer, 0, 0, sizeof(VertexType), (GLsizei)(vert_buffer.used_capacity() / sizeof(VertexType)));
		VAO.attach_element_buffer(index_buffer, (GLsizei)(index_buffer.used_capacity() / sizeof(unsigned int)));

		// const size_t chunk_stride = chunk_vert_buff_stride();
		// const size_t chunk_offset = data_index * chunk_stride;
		// LOG("[TERRAIN] Adding node {} vert data with bounds {}, {} -> {}, {}. Buffer section added from {}B to {}B", data_index, bounds.min.x, bounds.min.y, bounds.max.x, bounds.max.y, chunk_offset, chunk_offset + chunk_stride);
	}

	Terrain::Terrain(float height) noexcept
		: VAO{}
		, vert_buffer{{OpenGL::BufferStorageFlag::DynamicStorageBit}}
		, index_buffer{{OpenGL::BufferStorageFlag::DynamicStorageBit}}
		, node_mesh_info{}
		, root_bounds{}
		, max_depth{6}
		, chunk_detail{256}
		, decay_rate{0.006f}
		, noise_params{}
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

		noise_params.height = height;
	}
	Terrain::Terrain(const Terrain& p_other)
		: VAO{}
		, vert_buffer{p_other.vert_buffer}
		, index_buffer{p_other.index_buffer}
		, node_mesh_info{p_other.node_mesh_info}
		, root_bounds{p_other.root_bounds}
		, max_depth{p_other.max_depth}
		, chunk_detail{p_other.chunk_detail}
		, decay_rate{p_other.decay_rate}
		, noise_params{p_other.noise_params}
		, m_grass_tex{p_other.m_grass_tex}
		, m_gravel_tex{p_other.m_gravel_tex}
		, m_ground_tex{p_other.m_ground_tex}
		, m_rock_tex{p_other.m_rock_tex}
		, m_sand_tex{p_other.m_sand_tex}
		, m_snow_tex{p_other.m_snow_tex}
		, m_seed{p_other.m_seed}
	{
		constexpr GLint vertex_buffer_binding_point = 0;
		VAO.set_vertex_attrib_pointers(OpenGL::PrimitiveMode::Triangles, {
			{0, 3, OpenGL::BufferDataType::Float, offsetof(VertexType, position), vertex_buffer_binding_point, false},
			{1, 3, OpenGL::BufferDataType::Float, offsetof(VertexType, normal),   vertex_buffer_binding_point, false},
			{3, 2, OpenGL::BufferDataType::Float, offsetof(VertexType, uv),       vertex_buffer_binding_point, false},
			{2, 4, OpenGL::BufferDataType::Float, offsetof(VertexType, colour),   vertex_buffer_binding_point, false}
		});
		VAO.attach_buffer(vert_buffer, 0, 0, sizeof(VertexType), (GLsizei)(vert_buffer.used_capacity() / sizeof(VertexType)));
		VAO.attach_element_buffer(index_buffer, (GLsizei)(index_buffer.used_capacity() / sizeof(unsigned int)));
	}
	Terrain& Terrain::operator=(const Terrain& p_other)
	{(void)p_other;
		ASSERT_FAIL("ECS Storage does not support non-copyable components. Until support is added, this component must be copyable.");
	}

	void Terrain::update(const glm::vec3& player_pos_3D, float view_distance)
	{
		PERF(TerrainUpdate);

		player_pos = glm::vec2{player_pos_3D.x, player_pos_3D.z};
		if (!root_bounds)
			root_bounds = { player_pos, view_distance * 2.f };
		else if (root_bounds->half_size < view_distance)
			LOG_WARN(false, "Player view distance exceeds the half-size of the Terrain root, breaks invariant in Terrain::Update! Grow the terrain root to account for new view_distance");

		// Move the root of the tree in the exit direction of the player.
		// We impose the rule that the player view-sphere radius is always smaller than the root half size.
		// Therefore we can assert which of the corners or mid-points of the root edge we should move the root to based on the player
		// position inside a sub-quarter of the root.
		{
			constexpr Geometry::QuadKey RootTopLeft     { {Geometry::QuadKey::Quadrant::TopLeft} };
			constexpr Geometry::QuadKey RootTopRight    { {Geometry::QuadKey::Quadrant::TopRight} };
			constexpr Geometry::QuadKey RootBottomLeft  { {Geometry::QuadKey::Quadrant::BottomLeft} };
			constexpr Geometry::QuadKey RootBottomRight { {Geometry::QuadKey::Quadrant::BottomRight} };

			auto delete_contained_by = [&] (Geometry::QuadKey container)
			{
				//LOG("[TERRAIN] Deleting all nodes contained by {}", container.to_string());

				for (auto it = node_mesh_info.begin(); it != node_mesh_info.end();)
				{
					if (it->first.isContainedBy(container) || it->first == container)
					{
						//LOG("[TERRAIN] Removing quadkey {}", it->first.to_string());
						this->remove_verts(it);
					}
					else
						++it;
				}
			};
			auto reparent = [&](Geometry::QuadKey parent_container, Geometry::QuadKey::Quadrant new_parent)
			{
				for (auto it = node_mesh_info.begin(); it != node_mesh_info.end();)
				{
					if (it->first.isContainedBy(parent_container) || it->first == parent_container)
					{
						auto node = node_mesh_info.extract(it++);
						auto new_key = node.key().remap_root_quadrant(new_parent);
						//LOG("[TERRAIN] Reparenting quadkey {} to new parent {}.", node.key().to_string(), new_key.to_string());
						node.key() = new_key;
						ASSERT(node_mesh_info.find(node.key()) == node_mesh_info.end(), "Node with remapped key already exists in the mesh info map. Delete the old node before reparenting to ensure the mesh data is not corrupted.");
						node_mesh_info.insert(std::move(node));
					}
					else
						++it;
				}
			};

			auto x_max = root_bounds->center.x + (root_bounds->half_size * 0.5f);
			auto x_min = root_bounds->center.x - (root_bounds->half_size * 0.5f);
			auto y_max = root_bounds->center.y + (root_bounds->half_size * 0.5f);
			auto y_min = root_bounds->center.y - (root_bounds->half_size * 0.5f);

			if (player_pos.x < x_min) // Move left
			{
				LOG("[TERRAIN] Moving root left");
				root_bounds->center = { root_bounds->center.x - root_bounds->half_size, root_bounds->center.y };
				delete_contained_by(RootTopRight);
				delete_contained_by(RootBottomRight);
				reparent(RootTopLeft, Geometry::QuadKey::Quadrant::TopRight);
				reparent(RootBottomLeft, Geometry::QuadKey::Quadrant::BottomRight);
			}
			else if (player_pos.x > x_max) // Move right
			{
				LOG("[TERRAIN] Moving root right");
				root_bounds->center = { root_bounds->center.x + root_bounds->half_size, root_bounds->center.y };
				delete_contained_by(RootTopLeft);
				delete_contained_by(RootBottomLeft);
				reparent(RootTopRight, Geometry::QuadKey::Quadrant::TopLeft);
				reparent(RootBottomRight, Geometry::QuadKey::Quadrant::BottomLeft);
			}

			if (player_pos.y < y_min) // Move down
			{
				LOG("[TERRAIN] Moving root down");
				root_bounds->center = { root_bounds->center.x, root_bounds->center.y - root_bounds->half_size };
				delete_contained_by(RootTopLeft);
				delete_contained_by(RootTopRight);
				reparent(RootBottomLeft, Geometry::QuadKey::Quadrant::TopLeft);
				reparent(RootBottomRight, Geometry::QuadKey::Quadrant::TopRight);
			}
			else if (player_pos.y > y_max) // Move up
			{
				LOG("[TERRAIN] Moving root up");
				root_bounds->center = { root_bounds->center.x, root_bounds->center.y + root_bounds->half_size };
				delete_contained_by(RootBottomLeft);
				delete_contained_by(RootBottomRight);
				reparent(RootTopLeft, Geometry::QuadKey::Quadrant::BottomLeft);
				reparent(RootTopRight, Geometry::QuadKey::Quadrant::BottomRight);
			}
		}

		std::vector<Geometry::QuadKey> to_add_quads;
		auto to_remove_keys = node_mesh_info;

		auto leaf_quadkeys = get_tree_leaf_nodes();
		for (const auto& quadkey : leaf_quadkeys)
		{
			auto it = node_mesh_info.find(quadkey);

			if (it == node_mesh_info.end())
				to_add_quads.push_back(quadkey);
			else
				to_remove_keys.erase(quadkey);
		}

		if (!to_add_quads.empty() || !to_remove_keys.empty())
		{
			{// Figure out the required vert buffer size to accommodate the new verts.
				PERF(ResizeVertBuffer);
				long long int size_change = (chunk_vert_buff_stride() * to_add_quads.size()) - (to_remove_keys.size() * chunk_vert_buff_stride());

				if (size_change > 0 && vert_buffer.capacity() < vert_buffer.used_capacity() + size_change)
				{
					const size_t new_capacity = Utility::next_power_of_2(vert_buffer.used_capacity() + size_change);
					auto current_cap          = Utility::format_number(vert_buffer.used_capacity());
					auto new_cap_formatted    = Utility::format_number(new_capacity);
					LOG("[TERRAIN] Resizing terrain buffer from {}B to {}B", current_cap, new_cap_formatted);
					vert_buffer.reserve(new_capacity);
				}
			}

			for (const auto& quadkey : to_add_quads)
			{
				if (!to_remove_keys.empty())
				{
					auto to_remove_info = to_remove_keys.begin();
					add_verts(quadkey, to_remove_info->second);
					node_mesh_info.erase(to_remove_info->first);
					to_remove_keys.erase(to_remove_keys.begin());
				}
				else
					add_verts(quadkey);
			}

			for (const auto& to_remove_info : to_remove_keys)
			{
				auto it = node_mesh_info.find(to_remove_info.first);
				remove_verts(it);
			}
		}
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
			changed |= ImGui::Slider("Scale", noise_params.scale, 1.0f, 1000.0f);
			changed |= ImGui::Slider("Octaves", noise_params.octaves, 1u, 10u);
			ImGui::HelpMarker("Octaves are the number of layers of noise. More octaves means more detail, but also more computation.");
			changed |= ImGui::Slider("Persistence", noise_params.persistence, 0.01f, 3.0f);
			ImGui::HelpMarker("Persistence is the amplitude of each octave. A value of 1 means each octave has the same amplitude, while a value of 0.5 means each octave has half the amplitude of the previous one.");
			changed |= ImGui::Slider("Lacunarity", noise_params.lacunarity, 0.01f, 4.0f);
			ImGui::HelpMarker("Lacunarity is the frequency of each octave. A value of 1 means each octave has the same frequency, while a value of 2 means each octave has double the frequency of the previous one.");
			changed |= ImGui::Slider("Exponentiation", noise_params.exponentiation, 0.01f, 10.0f);
			ImGui::HelpMarker("Exponentiation is the power to which the noise value is raised. Effectively higher values will make the terrain more extreme, while lower values will make it smoother.");
			changed |= ImGui::Slider("Height", noise_params.height, 1.0f, 2560.0f);
			ImGui::HelpMarker("Height is the maximum height of the terrain. This is multiplied by the noise value to get the final height.");
			changed |= ImGui::InputScalar("Seed", ImGuiDataType_U32, &m_seed);
			ImGui::SameLine();
			if (ImGui::Button("Rand"))
			{
				changed = true;
				m_seed  = Utility::get_random_number<unsigned int>();
			}
			{
				const char* items[] = {"Analytical normals", "Old normals"};
				static int current_item = gen_normals_analytically ? 0 : 1;
				if (ImGui::Combo("Normal generation method", &current_item, items, IM_ARRAYSIZE(items)))
				{
					gen_normals_analytically = (current_item == 0);
					changed = true;
				}
			}
			ImGui::SeparatorText("LOD tree settings");
			int im_chunk_detail = chunk_detail;
			using chunk_detail_t = decltype(chunk_detail);
			if (ImGui::SliderInt("Chunk detail", &im_chunk_detail, 1, std::numeric_limits<chunk_detail_t>::max()))
			{
				chunk_detail = static_cast<chunk_detail_t>(im_chunk_detail);
				changed      = true;
			}
			changed |= ImGui::Slider("Max depth", max_depth, 0, 16);
			changed |= ImGui::Slider("Decay rate", decay_rate, 0.000001f, 1.f, "%.6f", ImGuiSliderFlags_Logarithmic);

			auto draw_2D_bounds = [&](const Geometry::AABB2D& bounds, const glm::vec4& col)
			{
				float y_offset = -1.f;
				OpenGL::DebugRenderer::add(Geometry::LineSegment{glm::vec3{bounds.min.x, y_offset, bounds.min.y}, glm::vec3{bounds.min.x, y_offset, bounds.max.y}}, col); // Left edge
				OpenGL::DebugRenderer::add(Geometry::LineSegment{glm::vec3{bounds.min.x, y_offset, bounds.max.y}, glm::vec3{bounds.max.x, y_offset, bounds.max.y}}, col); // Top edge
				OpenGL::DebugRenderer::add(Geometry::LineSegment{glm::vec3{bounds.max.x, y_offset, bounds.max.y}, glm::vec3{bounds.max.x, y_offset, bounds.min.y}}, col); // Right edge
				OpenGL::DebugRenderer::add(Geometry::LineSegment{glm::vec3{bounds.max.x, y_offset, bounds.min.y}, glm::vec3{bounds.min.x, y_offset, bounds.min.y}}, col); // Bottom edge
			};

			ImGui::SeparatorText("Tree data");
			ImGui::Text("Active nodes", node_mesh_info.size());

			if (ImGui::TreeNode("Tree leaves"))
			{
				for (const auto& node : node_mesh_info)
				{
					std::string node_title = std::format("Node {} - {}", node.first.key, node.first.depth);
					if (ImGui::TreeNode(node_title.c_str()))
					{
						Geometry::AABB2D bounds = node.first.get_bounds(root_bounds->half_size, root_bounds->center);
						ImGui::Text("Bounds: Min(%.2f, %.2f), Max(%.2f, %.2f)", bounds.min.x, bounds.min.y, bounds.max.x, bounds.max.y);
						ImGui::Text("Center: (%.2f, %.2f)", (bounds.min.x + bounds.max.x) / 2.f, (bounds.min.y + bounds.max.y) / 2.f);
						ImGui::Text("Size: (%.2f, %.2f)", bounds.size().x, bounds.size().y);
						ImGui::Text("Quadkey (decimal)", node.first.key);
						ImGui::Text("Quadkey (binary)", std::bitset<64>(node.first.key).to_string().c_str());
						draw_2D_bounds(bounds, {1.f, 0.f, 0.f, 1.f});
						ImGui::TreePop();
					}
				}
				ImGui::TreePop();
			}

			ImGui::Text("Max depth", max_depth);
			ImGui::Text("Per node detail", (int)chunk_detail);
			ImGui::Text("Vert count ", Utility::format_number(vert_buffer.used_capacity() / sizeof(VertexType), 1));
			ImGui::Text("Index count", Utility::format_number(index_buffer.used_capacity() / sizeof(unsigned int), 1));
			ImGui::Text("Vert buffer size", Utility::format_number(vert_buffer.used_capacity(), 1) + "B");
			ImGui::Text("Index buffer size", Utility::format_number(index_buffer.used_capacity(), 1) + "B");
			ImGui::Text("Draw count", VAO.draw_count());

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