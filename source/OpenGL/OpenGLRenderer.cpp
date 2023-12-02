#include "OpenGLRenderer.hpp"
#include "DebugRenderer.hpp"
#include "DrawCall.hpp"

#include "ECS/Storage.hpp"
#include "Component/Lights.hpp"
#include "Component/Texture.hpp"
#include "Component/Collider.hpp"
#include "Component/Camera.hpp"
#include "Component/Mesh.hpp"
#include "Component/Transform.hpp"
#include "Component/Terrain.hpp"
#include "System/MeshSystem.hpp"
#include "System/TextureSystem.hpp"
#include "System/SceneSystem.hpp"

#include "Utility/Logger.hpp"
#include "Utility/Utility.hpp"
#include "Utility/Config.hpp"
#include "Utility/MeshBuilder.hpp"
#include "Platform/Core.hpp"
#include "Platform/Window.hpp"

#include "glad/gl.h"

namespace OpenGL
{
	Data::Mesh OpenGLRenderer::make_screen_quad_mesh()
	{
		auto mb = Utility::MeshBuilder<Data::TextureVertex, OpenGL::PrimitiveMode::Triangles>{};
		mb.add_quad(glm::vec3(-1.f, 1.f, 0.f), glm::vec3(1.f, 1.f, 0.f), glm::vec3(-1.f, -1.f, 0.f), glm::vec3(1.f, -1.f, 0.f));
		return mb.get_mesh();
	}

	OpenGLRenderer::OpenGLRenderer(Platform::Window& p_window, System::SceneSystem& p_scene_system, System::MeshSystem& p_mesh_system, System::TextureSystem& p_texture_system) noexcept
		: m_window{p_window}
		, m_screen_framebuffer{}
		, m_scene_system{p_scene_system}
		, m_mesh_system{p_mesh_system}
		, m_uniform_colour_shader{"uniformColour"}
		, m_colour_shader{"colour"}
		, m_texture_shader{"texture1"}
		, m_screen_texture_shader{"screenTexture"}
		, m_sky_box_shader{"skybox"}
		, m_phong_renderer{}
		, m_particle_renderer{}
		, m_shadow_mapper{p_window}
		, m_missing_texture{p_texture_system.m_texture_manager.insert(Data::Texture{Config::Texture_Directory / "missing.png"})}
		, m_blank_texture{p_texture_system.m_texture_manager.insert(Data::Texture{Config::Texture_Directory / "black.jpg"})}
		, m_screen_quad{make_screen_quad_mesh()}
		, m_view_information{}
		, m_post_processing_options{}
	{
		const auto windowSize = m_window.size();
		m_screen_framebuffer.attachColourBuffer(windowSize.x, windowSize.y);
		m_screen_framebuffer.attachDepthBuffer(windowSize.x, windowSize.y);
		set_viewport(0, 0, windowSize.x, windowSize.y);
		LOG("[OPENGL] Constructed new OpenGLRenderer instance");
	}

	void OpenGLRenderer::start_frame()
	{
		{ // Set global shader uniforms.
			m_scene_system.get_current_scene().foreach([this](Component::Camera& p_camera, Component::Transform& p_transform)
			{
				if (p_camera.m_primary)
				{
					m_view_information.m_view_position = p_transform.m_position;
					m_view_information.m_view         = p_camera.view(p_transform.m_position);// glm::lookAt(p_transform.m_position, p_transform.m_position + p_transform.m_direction, camera_up);
					m_view_information.m_projection   = glm::perspective(glm::radians(p_camera.m_FOV), m_window.aspect_ratio(), p_camera.m_near, p_camera.m_far);

					Shader::set_block_uniform("ViewProperties.view", m_view_information.m_view);
					Shader::set_block_uniform("ViewProperties.projection", m_view_information.m_projection);
				}
			});
		}

		FBO::unbind();

		m_shadow_mapper.shadow_pass(m_scene_system.m_scene);

		{ // Prepare m_screen_framebuffer for rendering
			const auto window_size = m_window.size();
			m_screen_framebuffer.resize(window_size.x, window_size.y);
			set_viewport(0, 0, window_size.x, window_size.y);
			m_screen_framebuffer.bind();
			m_screen_framebuffer.clearBuffers();
			ASSERT(m_screen_framebuffer.isComplete(), "Screen framebuffer not complete, have you attached a colour or depth buffer to it?");
		}
	}

	void OpenGLRenderer::draw(const DeltaTime& delta_time)
	{
		m_phong_renderer.update_light_data(m_scene_system.m_scene, m_shadow_mapper.get_depth_map());
		auto& scene = m_scene_system.get_current_scene();

		scene.foreach([&](ECS::Entity& p_entity, Component::Transform& p_transform, Component::Mesh& mesh_comp)
		{
			if (scene.has_components<Component::Texture>(p_entity))
			{
				auto& texComponent = scene.get_component<Component::Texture>(p_entity);

				DrawCall dc;
				dc.set_uniform("view_position", m_view_information.m_view_position);
				dc.set_uniform("model", p_transform.m_model);
				dc.set_uniform("shininess", texComponent.m_shininess);
				dc.set_texture("diffuse",  texComponent.m_diffuse.has_value()  ? texComponent.m_diffuse  : m_missing_texture);
				dc.set_texture("specular", texComponent.m_specular.has_value() ? texComponent.m_specular : m_blank_texture);
				dc.submit(m_phong_renderer.get_shader(), mesh_comp.m_mesh);
			}
			else
			{
				DrawCall dc;
				dc.set_uniform("model", p_transform.m_model);
				dc.set_uniform("colour", glm::vec4(0.06f, 0.44f, 0.81f, 1.f));
				dc.submit(m_uniform_colour_shader, mesh_comp.m_mesh);
			}
		});

		{// Draw terrain
			scene.foreach([&](Component::Terrain& p_terrain)
			{
				DrawCall dc;
				dc.set_uniform("view_position", m_view_information.m_view_position);
				dc.set_uniform("model", glm::translate(glm::identity<glm::mat4>(), p_terrain.m_position));
				dc.set_uniform("shininess", 64.f);
				dc.set_texture("diffuse", p_terrain.m_texture.has_value() ? p_terrain.m_texture : m_missing_texture);
				dc.set_texture("specular",  m_blank_texture);
				dc.submit(m_phong_renderer.get_shader(), p_terrain.m_mesh);
			});
		}

		m_particle_renderer.update(delta_time, m_scene_system.m_scene, m_view_information.m_view_position);
	}

	void OpenGLRenderer::end_frame()
	{
		{ // Draw the colour output to the from m_screen_framebuffer texture to the default FBO
			// Unbind after completing draw to ensure all subsequent actions apply to the default FBO and not mScreenFrameBuffer.
			// Disable depth testing to not cull the screen quad the screen texture will be applied onto.
			FBO::unbind();
			set_depth_test(false);
			set_cull_face(false);
			set_polygon_mode(PolygonMode::Fill);
			glClear(GL_COLOR_BUFFER_BIT);

			m_screen_texture_shader.use();
			{ // PostProcessing setters
				m_screen_texture_shader.set_uniform("invertColours", m_post_processing_options.mInvertColours);
				m_screen_texture_shader.set_uniform("grayScale", m_post_processing_options.mGrayScale);
				m_screen_texture_shader.set_uniform("sharpen", m_post_processing_options.mSharpen);
				m_screen_texture_shader.set_uniform("blur", m_post_processing_options.mBlur);
				m_screen_texture_shader.set_uniform("edgeDetection", m_post_processing_options.mEdgeDetection);
				m_screen_texture_shader.set_uniform("offset", m_post_processing_options.mKernelOffset);
			}

			active_texture(0);
			m_screen_framebuffer.bindColourTexture();
			m_screen_quad.draw();
		}
	}
} // namespace OpenGL