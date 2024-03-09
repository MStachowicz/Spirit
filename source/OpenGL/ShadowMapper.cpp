#include "ShadowMapper.hpp"
#include "DrawCall.hpp"

#include "Component/FirstPersonCamera.hpp"
#include "Component/Lights.hpp"
#include "Component/Mesh.hpp"
#include "Component/Transform.hpp"
#include "ECS/Storage.hpp"
#include "Platform/Window.hpp"
#include "System/SceneSystem.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/mat4x4.hpp"

namespace OpenGL
{
	ShadowMapper::ShadowMapper(Platform::Window& p_window) noexcept
		: m_depth_map_FBO{}
		, m_shadow_depth_shader{"shadowDepth"}
		, m_resolution{glm::uvec2(1024 * 4)}
		, m_window{p_window}
	{
		m_depth_map_FBO.attach_depth_buffer(m_resolution);
	}

	void ShadowMapper::shadow_pass(System::Scene& p_scene)
	{
		ASSERT(m_depth_map_FBO.isComplete(), "[OPENGL][SHADOW MAPPER] framebuffer not complete, have you attached a depth buffer + empty draw and read buffers.");

		unsigned int directional_light_count = 0;
		p_scene.m_entities.foreach([&directional_light_count](Component::DirectionalLight& p_light) { (void)p_light; directional_light_count++; }); // #TODO replace with a storage::count<>()

		if (directional_light_count > 0)
		{
			m_depth_map_FBO.bind();
			set_viewport(0, 0, m_resolution.x, m_resolution.y);
			m_depth_map_FBO.clearBuffers();

			// Draw the scene from the perspective of the light
			p_scene.m_entities.foreach([&](Component::DirectionalLight& p_light)
			{
				p_scene.m_entities.foreach([&](Component::Transform& p_transform, Component::Mesh& p_mesh)
				{
					DrawCall dc;
					dc.m_cull_face_enabled = false;
					dc.set_uniform("light_space_mat", p_light.get_view_proj(p_scene.m_bound));
					dc.set_uniform("model", p_transform.m_model);
					dc.submit(m_shadow_depth_shader, p_mesh.m_mesh);
				});
			});
			m_depth_map_FBO.unbind();
		}
	}

	void ShadowMapper::draw_UI()
	{

	}
} // namespace OpenGL