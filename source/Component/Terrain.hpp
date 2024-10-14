#pragma once

#include "Component/Texture.hpp"
#include "Component/Mesh.hpp"

#include "Utility/PerlinNoise.hpp"



namespace System
{
	class AssetManager;
}
namespace Component
{
	class Terrain
	{
		Data::Mesh generate_mesh(unsigned int p_seed) noexcept;

	public:
		constexpr static size_t Persistent_ID = 6;

		glm::vec3 m_position;
		int m_size_x;
		int m_size_z;
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
		Data::Mesh m_mesh;

		Terrain(const glm::vec3& p_position, int p_size_x, int p_size_z, float amplitude) noexcept;
		// Copy constructor has to be implemented because Data::Mesh GL members are not copyable.
		Terrain(const Terrain& p_other) noexcept;
		// Copy assignment operator has to be implemented because Data::Mesh GL members are not copyable.
		Terrain& operator=(const Terrain& p_other) noexcept;
		Terrain(Terrain&& p_other) noexcept            = default;
		Terrain& operator=(Terrain&& p_other) noexcept = default;

		float compute_height(float p_x, float p_z, const siv::PerlinNoise& perlin);
		void draw_UI(System::AssetManager& p_asset_manager);
	};
} // namespace Component