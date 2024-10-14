#include "Terrain.hpp"

#include "System/AssetManager.hpp"
#include "Utility/MeshBuilder.hpp"
#include "Utility/PerlinNoise.hpp"
#include "Utility/Stopwatch.hpp"

#include "imgui.h"

Component::Terrain::Terrain(const glm::vec3& p_position, int p_size_x, int p_size_z, float amplitude) noexcept
	: m_position{p_position}
	, m_size_x{p_size_x}
	, m_size_z{p_size_z}
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
	, m_mesh{generate_mesh(m_seed)}
{}
Component::Terrain::Terrain(const Terrain& p_other) noexcept
	: m_position{p_other.m_position}
	, m_size_x{p_other.m_size_x}
	, m_size_z{p_other.m_size_z}
	, m_scale_factor{p_other.m_scale_factor}
	, m_amplitude{p_other.m_amplitude}
	, m_lacunarity{p_other.m_lacunarity}
	, m_persistence{p_other.m_persistence}
	, m_octaves{p_other.m_octaves}
	, m_grass_tex{p_other.m_grass_tex}
	, m_gravel_tex{p_other.m_gravel_tex}
	, m_ground_tex{p_other.m_ground_tex}
	, m_rock_tex{p_other.m_rock_tex}
	, m_sand_tex{p_other.m_sand_tex}
	, m_snow_tex{p_other.m_snow_tex}
	, m_seed{p_other.m_seed}
	, m_mesh{generate_mesh(m_seed)} // TODO: Implement a Data::Mesh copy.
{}
Component::Terrain& Component::Terrain::operator=(const Terrain& p_other) noexcept
{
	m_position     = p_other.m_position;
	m_size_x       = p_other.m_size_x;
	m_size_z       = p_other.m_size_z;
	m_scale_factor = p_other.m_scale_factor;
	m_amplitude    = p_other.m_amplitude;
	m_lacunarity   = p_other.m_lacunarity;
	m_persistence  = p_other.m_persistence;
	m_octaves      = p_other.m_octaves;
	m_grass_tex    = p_other.m_grass_tex;
	m_gravel_tex   = p_other.m_gravel_tex;
	m_ground_tex   = p_other.m_ground_tex;
	m_rock_tex     = p_other.m_rock_tex;
	m_sand_tex     = p_other.m_sand_tex;
	m_snow_tex     = p_other.m_snow_tex;
	m_seed         = p_other.m_seed;
	m_mesh         = generate_mesh(m_seed); // TODO: Implement a Data::Mesh copy.
	return *this;
}

float Component::Terrain::compute_height(float p_x, float p_z, const siv::PerlinNoise& perlin)
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

Data::Mesh Component::Terrain::generate_mesh(unsigned int p_seed) noexcept
{
	const siv::PerlinNoise perlin{p_seed};
	std::vector<Data::Vertex> unique_verts;
	std::vector<unsigned int> indices;

	for (int z = 0; z <= m_size_z; ++z)
	{
		for (int x = 0; x <= m_size_x; ++x)
		{
			Data::Vertex vert;
			auto y = compute_height((float)x, (float)z, perlin);
			vert.position = {x, y, z};
			vert.normal   = {0.f, 0.f, 0.f}; // Calculate normals later
			unique_verts.push_back(vert);
		}
	}

	// Generate indices for each quad (two triangles per quad)
	for (int z = 0; z < m_size_z; ++z)
	{
		for (int x = 0; x < m_size_x; ++x)
		{
			// Get the indices for the four corners of the current quad
			unsigned int top_left     = z * (m_size_x + 1) + x;
			unsigned int top_right    = top_left + 1;
			unsigned int bottom_left  = (z + 1) * (m_size_x + 1) + x;
			unsigned int bottom_right = bottom_left + 1;

			// First triangle (top-left, bottom-left, top-right)
			indices.push_back(top_left);
			indices.push_back(bottom_left);
			indices.push_back(top_right);
			// Second triangle (top-right, bottom-left, bottom-right)
			indices.push_back(top_right);
			indices.push_back(bottom_left);
			indices.push_back(bottom_right);

			// Calculate normals for the first triangle then accumulate the normal to each of the triangle's vertices
			glm::vec3 v0     = unique_verts[top_left].position;
			glm::vec3 v1     = unique_verts[bottom_left].position;
			glm::vec3 v2     = unique_verts[top_right].position;
			glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
			unique_verts[top_left].normal    += normal;
			unique_verts[bottom_left].normal += normal;
			unique_verts[top_right].normal   += normal;

			// Calculate normals for the second triangle
			// Accumulate the normal to each of the triangle's vertices
			v0     = unique_verts[top_right].position;
			v1     = unique_verts[bottom_left].position;
			v2     = unique_verts[bottom_right].position;
			normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
			unique_verts[top_right].normal    += normal;
			unique_verts[bottom_left].normal  += normal;
			unique_verts[bottom_right].normal += normal;

			// Calculate UV coordinates based on vertex position within the terrain grid.
			// UV coordinates range from 0.0 to size_x or size_z
			float u = static_cast<float>(x);
			float v = static_cast<float>(z);
			unique_verts[top_left].uv     = {u, v + 1.0f};
			unique_verts[bottom_left].uv  = {u, v};
			unique_verts[top_right].uv    = {u + 1.0f, v + 1.0f};
			unique_verts[bottom_right].uv = {u + 1.0f, v};
		}
	}

	// Normalize the vertex normals
	for (auto& vert : unique_verts)
		vert.normal = glm::normalize(vert.normal);

	return Data::Mesh{std::move(unique_verts), std::move(indices), OpenGL::PrimitiveMode::Triangles};
}

void Component::Terrain::draw_UI(System::AssetManager& p_asset_manager)
{
	if (ImGui::TreeNode("Terrain"))
	{
		m_mesh.draw_UI();

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
		changed |= ImGui::Slider("Position", m_position, -100.f, 100.f, "%.3fm");
		changed |= ImGui::Slider("Size X", m_size_x, 1, 1000, "%dm");
		changed |= ImGui::Slider("Size Z", m_size_z, 1, 1000, "%dm");
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
			m_mesh = generate_mesh(m_seed);
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