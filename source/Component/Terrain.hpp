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
		glm::vec3 m_position;
		int m_size_x;
		int m_size_z;
		float m_scale_factor;
		TextureRef m_texture;
		Data::Mesh m_mesh;

		Terrain(int p_size_x, int p_size_z) noexcept;
		void draw_UI(System::TextureSystem& p_texture_system);
	};
} // namespace Component