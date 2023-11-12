#include "Types.hpp"
#include "OpenGL/Shader.hpp"
#include "Component/Mesh.hpp"

namespace ECS
{
	class Storage;
}
namespace OpenGL
{
	class LightPositionRenderer
	{
		static Data::Mesh make_point_light_mesh();

		Shader m_light_position_shader;
		float m_light_position_scale;
		Data::Mesh m_point_light_mesh;

	public:
		LightPositionRenderer() noexcept;

		void draw(ECS::Storage& p_storage);
	};
}