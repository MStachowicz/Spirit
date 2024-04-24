#include "ShadowMapper.hpp"
#include "DrawCall.hpp"

#include "Component/Lights.hpp"
#include "Component/Mesh.hpp"
#include "Component/Transform.hpp"
#include "ECS/Storage.hpp"
#include "System/SceneSystem.hpp"

namespace OpenGL
{
	ShadowMapper::ShadowMapper(const glm::uvec2& p_resolution) noexcept
		: m_depth_map_FBO{p_resolution, false, true, false}
		, m_shadow_depth_shader{"shadowDepth"}
	{}

	void ShadowMapper::shadow_pass(System::Scene& p_scene)
	{
		m_depth_map_FBO.clear();
		unsigned int directional_light_count = static_cast<unsigned int>(p_scene.m_entities.count_components<Component::DirectionalLight>());

		if (directional_light_count > 0)
		{
			// Draw the scene from the perspective of the light
			p_scene.m_entities.foreach([&](Component::DirectionalLight& p_light)
			{
				p_scene.m_entities.foreach([&](Component::Transform& p_transform, Component::Mesh& p_mesh)
				{
					DrawCall dc;
					dc.m_cull_face_enabled = false;
					dc.m_depth_test_enabled = true;
					dc.m_write_to_depth_buffer = true;
					dc.m_depth_test_type = DepthTestType::Less;
					dc.set_uniform("light_space_mat", p_light.get_view_proj(p_scene.m_bound));
					dc.set_uniform("model", p_transform.get_model());
					dc.submit(m_shadow_depth_shader, p_mesh.m_mesh->get_VAO(), m_depth_map_FBO);
				});
			});
		}
	}

	void ShadowMapper::draw_UI()
	{}
} // namespace OpenGL