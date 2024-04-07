#pragma once

#include "Component/Texture.hpp"
#include "Component/Mesh.hpp"

namespace System
{
	class TextureSystem;
}
namespace Component
{
	class Terrain
	{
		Data::Mesh generate_mesh() noexcept;

	public:
		constexpr static size_t Persistent_ID = 6;

		glm::vec3 m_position;
		int m_size_x;
		int m_size_z;
		float m_scale_factor;
		TextureRef m_texture;
		Data::Mesh m_mesh;

		Terrain(const glm::vec3& p_position, int p_size_x, int p_size_z) noexcept;
		// Copy constructor has to be implemented because Data::Mesh GL members are not copyable.
		Terrain(const Terrain& p_other) noexcept;
		// Copy assignment operator has to be implemented because Data::Mesh GL members are not copyable.
		Terrain& operator=(const Terrain& p_other) noexcept;
		Terrain(Terrain&& p_other) noexcept            = default;
		Terrain& operator=(Terrain&& p_other) noexcept = default;

		void draw_UI(System::TextureSystem& p_texture_system);
	};
} // namespace Component