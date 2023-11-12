#include "LightPositionRenderer.hpp"
#include "OpenGL/GLState.hpp"
#include "OpenGL/DrawCall.hpp"

#include "Component/Lights.hpp"
#include "ECS/Storage.hpp"
#include "Utility/MeshBuilder.hpp"

namespace OpenGL
{
	Data::Mesh LightPositionRenderer::make_point_light_mesh()
	{
		auto mb = Utility::MeshBuilder<Data::PositionVertex, OpenGL::PrimitiveMode::Triangles>{};
		mb.add_icosphere(glm::vec3(0.f), 1.f, 1);
		return mb.get_mesh();
	}

	LightPositionRenderer::LightPositionRenderer() noexcept
		: m_light_position_shader{"light_position"}
		, m_light_position_scale{0.25f}
		, m_point_light_mesh{make_point_light_mesh()}
	{}

	void LightPositionRenderer::draw(ECS::Storage& p_storage)
	{
		int point_light_count = 0;
		p_storage.foreach ([&point_light_count](Component::PointLight& point_light) { point_light_count++; });

		if (point_light_count > 0)
		{
			DrawCall dc;
			dc.set_uniform("scale", m_light_position_scale);
			dc.submit(m_light_position_shader, m_point_light_mesh, point_light_count);
		}
	}
}