#include "OpenGLRenderer.hpp"
#include "DebugRenderer.hpp"
#include "DrawCall.hpp"

#include "Component/Collider.hpp"
#include "Component/FirstPersonCamera.hpp"
#include "Component/Lights.hpp"
#include "Component/Terrain.hpp"
#include "Component/Transform.hpp"
#include "ECS/Storage.hpp"
#include "System/MeshSystem.hpp"
#include "System/SceneSystem.hpp"
#include "System/TextureSystem.hpp"

#include "Platform/Core.hpp"
#include "Platform/Window.hpp"
#include "Utility/Config.hpp"
#include "Utility/Logger.hpp"
#include "Utility/MeshBuilder.hpp"
#include "Utility/Utility.hpp"

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
		, m_screen_framebuffer{m_window.size()}
		, m_scene_system{p_scene_system}
		, m_mesh_system{p_mesh_system}
		, m_view_properties_buffer{{OpenGL::BufferStorageFlag::DynamicStorageBit}}
		, m_uniform_colour_shader{"uniformColour"}
		, m_colour_shader{"colour"}
		, m_texture_shader{"texture1"}
		, m_screen_texture_shader{"screenTexture"}
		, m_sky_box_shader{"skybox"}
		, m_phong_renderer{}
		, m_particle_renderer{}
		, m_grid_renderer{}
		, m_shadow_mapper{glm::uvec2(2048, 2048)}
		, m_missing_texture{p_texture_system.m_texture_manager.insert(Data::Texture{Config::Texture_Directory / "missing.png"})}
		, m_blank_texture{p_texture_system.m_texture_manager.insert(Data::Texture{Config::Texture_Directory / "black.jpg"})}
		, m_screen_quad{make_screen_quad_mesh()}
		, m_post_processing_options{}
	{
		#ifdef Z_DEBUG // Ensure the uniform block layout matches the Component::ViewInformation struct layout for direct memory copy.
		{
			auto view_properties_block = m_colour_shader.get_uniform_block("ViewProperties");
			ASSERT(view_properties_block.m_data_size == sizeof(Component::ViewInformation), "ViewProperties block size mismatch. Has the shader or the Component::ViewInformation struct changed?");
			ASSERT(view_properties_block.m_variables.size() == 3, "ViewProperties block variable count mismatch. Has the shader or the Component::ViewInformation struct changed?");

			ASSERT(view_properties_block.get_variable("ViewProperties.view").m_offset == offsetof(Component::ViewInformation, m_view)
			    && view_properties_block.get_variable("ViewProperties.view").m_type   == ShaderDataType::Mat4
			    , "ViewProperties.view block variable mismatch. Has the shader or the Component::ViewInformation struct changed?");

			ASSERT(view_properties_block.get_variable("ViewProperties.projection").m_offset == offsetof(Component::ViewInformation, m_projection)
			    && view_properties_block.get_variable("ViewProperties.projection").m_type   == ShaderDataType::Mat4
				, "ViewProperties.projection block variable mismatch. Has the shader or the Component::ViewInformation struct changed?");

			ASSERT(view_properties_block.get_variable("ViewProperties.camera_position").m_offset == offsetof(Component::ViewInformation, m_view_position)
			    && view_properties_block.get_variable("ViewProperties.camera_position").m_type   == ShaderDataType::Vec4
				, "ViewProperties.view_position block variable mismatch. Has the shader or the Component::ViewInformation struct changed?");
		}
		#endif
		m_view_properties_buffer.resize(sizeof(Component::ViewInformation));
		m_view_properties_buffer.buffer_sub_data(0, m_scene_system.get_current_scene_view_info());

		LOG("[OPENGL] Constructed new OpenGLRenderer instance");
	}

	void OpenGLRenderer::start_frame()
	{
		m_view_properties_buffer.buffer_sub_data(0, m_scene_system.get_current_scene_view_info());

		m_shadow_mapper.shadow_pass(m_scene_system.get_current_scene());

		// Prepare m_screen_framebuffer for rendering
		m_screen_framebuffer.resize(m_window.size());
		auto clear_colour = Platform::Core::s_theme.background;
		clear_colour.a    = 1.f;
		m_screen_framebuffer.set_clear_colour(clear_colour); // Platform::Core::s_theme.background);
		m_screen_framebuffer.clear();
		ASSERT(m_screen_framebuffer.is_complete(), "Screen framebuffer not complete, have you attached a colour or depth buffer to it?");
	}

	void OpenGLRenderer::draw(const DeltaTime& delta_time)
	{
		auto& entities = m_scene_system.get_current_scene_entities();
		auto& scene    = m_scene_system.get_current_scene();

		auto get_first_light_proj_view = [&]() -> glm::mat4
		{
			glm::mat4 proj_view = glm::identity<glm::mat4>();
			entities.foreach([&](Component::DirectionalLight& p_light)
				{
					proj_view = p_light.get_view_proj(scene.m_bound);
					return;
				});
			return proj_view;
		};

		m_grid_renderer.draw(m_screen_framebuffer);

		m_phong_renderer.update_light_data(m_scene_system.get_current_scene());
		const auto& directional_light_buffer = m_phong_renderer.get_directional_lights_buffer();
		const auto& point_light_buffer       = m_phong_renderer.get_point_lights_buffer();
		const auto& spot_light_buffer        = m_phong_renderer.get_spot_lights_buffer();

		entities.foreach([&](ECS::Entity& p_entity, Component::Transform& p_transform, Component::Mesh& mesh_comp)
		{
			if (mesh_comp.m_mesh)
			{
				if (entities.has_components<Component::Texture>(p_entity))
				{
					auto& texComponent = entities.get_component<Component::Texture>(p_entity);

					DrawCall dc;
					dc.set_uniform("model", p_transform.get_model());
					dc.set_uniform("light_proj_view", get_first_light_proj_view());
					dc.set_uniform("shininess", texComponent.m_shininess);
					dc.set_uniform("PCF_bias", Component::DirectionalLight::PCF_bias);

					dc.set_SSBO("DirectionalLightsBuffer", directional_light_buffer);
					dc.set_SSBO("PointLightsBuffer", point_light_buffer);
					dc.set_SSBO("SpotLightsBuffer", spot_light_buffer);

					dc.set_UBO("ViewProperties", m_view_properties_buffer);
					dc.set_texture("shadow_map", m_shadow_mapper.get_depth_map());

					if (texComponent.m_diffuse.has_value())
					{
						dc.set_texture("diffuse",  texComponent.m_diffuse->m_GL_texture);
						dc.set_texture("specular", texComponent.m_specular.has_value() ? texComponent.m_specular->m_GL_texture : m_blank_texture->m_GL_texture);
						dc.submit(m_phong_renderer.get_texture_shader(), mesh_comp.m_mesh->get_VAO(), m_screen_framebuffer);
					}
					else
					{
						dc.set_uniform("uColour", texComponent.m_colour);

						// Dont write transparent pixels to the depth buffer. This prevents transparent objects from culling other objects behind them.
						// TODO: Group all these together and draw in a single pass ordering them by distance to camera.
						if (texComponent.m_colour.a < 1.f)
						{
							dc.m_write_to_depth_buffer = false;
							dc.m_blending_enabled = true;
						}
						dc.submit(m_phong_renderer.get_uniform_colour_shader(), mesh_comp.m_mesh->get_VAO(), m_screen_framebuffer);
					}

					return;
				}

				// Fallback to rendering using default colour and no lighting.
				DrawCall dc;
				dc.set_uniform("model", p_transform.get_model());
				dc.set_uniform("colour", glm::vec4(0.06f, 0.44f, 0.81f, 1.f));
				dc.set_UBO("ViewProperties", m_view_properties_buffer);
				dc.submit(m_uniform_colour_shader, mesh_comp.m_mesh->get_VAO(), m_screen_framebuffer);
			}
		});

		{// Draw terrain
			entities.foreach([&](Component::Terrain& p_terrain)
			{
				DrawCall dc;
				dc.set_uniform("model", glm::translate(glm::identity<glm::mat4>(), p_terrain.m_position));
				dc.set_uniform("light_proj_view", get_first_light_proj_view());
				dc.set_uniform("shininess", 64.f);
				dc.set_uniform("PCF_bias", Component::DirectionalLight::PCF_bias);

				dc.set_SSBO("DirectionalLightsBuffer", directional_light_buffer);
				dc.set_SSBO("PointLightsBuffer", point_light_buffer);
				dc.set_SSBO("SpotLightsBuffer", spot_light_buffer);

				dc.set_UBO("ViewProperties", m_view_properties_buffer);
				dc.set_texture("shadow_map", m_shadow_mapper.get_depth_map());

				dc.set_texture("diffuse",  p_terrain.m_texture.has_value() ? p_terrain.m_texture->m_GL_texture : m_missing_texture->m_GL_texture);
				dc.set_texture("specular", m_blank_texture->m_GL_texture);
				dc.submit(m_phong_renderer.get_texture_shader(), p_terrain.m_mesh.get_VAO(), m_screen_framebuffer);
			});
		}

		m_particle_renderer.update(delta_time, m_scene_system.get_current_scene(), m_scene_system.get_current_scene_view_info().m_view_position, m_view_properties_buffer, m_screen_framebuffer);
	}

	void OpenGLRenderer::end_frame()
	{
		OpenGL::DebugRenderer::render(m_scene_system, m_view_properties_buffer, m_screen_framebuffer);

		// Draw the colour output from m_screen_framebuffer colour texture to the default FBO as a fullscreen quad.

		DrawCall dc;
		dc.m_depth_test_enabled    = false;
		dc.m_cull_face_enabled     = false;
		dc.m_polygon_mode          = PolygonMode::Fill;
		dc.m_write_to_depth_buffer = false;

		{ // PostProcessing setters
			dc.set_uniform("invertColours", m_post_processing_options.mInvertColours);
			dc.set_uniform("grayScale", m_post_processing_options.mGrayScale);
			dc.set_uniform("sharpen", m_post_processing_options.mSharpen);
			dc.set_uniform("blur", m_post_processing_options.mBlur);
			dc.set_uniform("edgeDetection", m_post_processing_options.mEdgeDetection);
			dc.set_uniform("offset", m_post_processing_options.mKernelOffset);
		}

		dc.set_texture("screen_texture", m_screen_framebuffer.color_attachment());

		dc.submit_default(m_screen_texture_shader, m_screen_quad.get_VAO(), m_window.size());
	}
} // namespace OpenGL