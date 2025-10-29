#include "OpenGLRenderer.hpp"
#include "DebugRenderer.hpp"
#include "DrawCall.hpp"

#include "Component/Collider.hpp"
#include "Component/FirstPersonCamera.hpp"
#include "Component/Lights.hpp"
#include "Component/Terrain.hpp"
#include "Component/Transform.hpp"
#include "ECS/Storage.hpp"
#include "System/AssetManager.hpp"
#include "System/SceneSystem.hpp"

#include "Platform/Core.hpp"
#include "Platform/Window.hpp"
#include "Utility/Config.hpp"
#include "Utility/Logger.hpp"
#include "Utility/MeshBuilder.hpp"
#include "Utility/Utility.hpp"
#include "Utility/Performance.hpp"

#include "imgui.h"

namespace OpenGL
{
	Data::Mesh make_screen_quad_mesh()
	{
		auto mb = Utility::MeshBuilder<Data::TextureVertex, OpenGL::PrimitiveMode::Triangles>{};
		mb.add_quad(glm::vec3(-1.f, 1.f, 0.f), glm::vec3(1.f, 1.f, 0.f), glm::vec3(-1.f, -1.f, 0.f), glm::vec3(1.f, -1.f, 0.f));
		return mb.get_mesh();
	}
	Data::Mesh make_axis_mesh()
	{
		constexpr size_t segments_per_cylinder = 8;
		constexpr float radius = 0.025f;
		auto mb = Utility::MeshBuilder<Data::ColourVertex, OpenGL::PrimitiveMode::Triangles>{};
		mb.set_colour(glm::vec3(1.f, 0.f, 0.f));
		mb.add_cylinder(glm::vec3(0.f), glm::vec3(1.f, 0.f, 0.f), radius, segments_per_cylinder); // X
		mb.set_colour(glm::vec3(0.f, 1.f, 0.f));
		mb.add_cylinder(glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f), radius, segments_per_cylinder); // Y
		mb.set_colour(glm::vec3(0.f, 0.f, 1.f));
		mb.add_cylinder(glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f), radius, segments_per_cylinder); // Z
		return mb.get_mesh();
	}

	OpenGLRenderer::OpenGLRenderer(System::AssetManager& p_asset_manager, System::SceneSystem& p_scene_system) noexcept
		: m_asset_manager{p_asset_manager}
		, m_scene_system{p_scene_system}
		, m_view_properties_buffer{{OpenGL::BufferStorageFlag::DynamicStorageBit}, sizeof(Component::ViewInformation)}
		, m_uniform_colour_shader{"uniformColour"}
		, m_colour_shader{"colour"}
		, m_texture_shader{"texture1"}
		, m_screen_texture_shader{"screenTexture"}
		, m_sky_box_shader{"skybox"}
		, m_terrain_shader{"phong_terrain"}
		, m_post_processing_FBO{}
		, m_phong_renderer{}
		, m_particle_renderer{}
		, m_grid_renderer{}
		, m_shadow_mapper{glm::uvec2(2048, 2048)}
		, m_missing_texture{m_asset_manager.get_texture("missing.png")}
		, m_blank_texture{m_asset_manager.get_texture("black.jpg")}
		, m_screen_quad{make_screen_quad_mesh()}
		, m_axis_mesh{make_axis_mesh()}
		, m_post_processing_options{}
		, m_draw_shadows{false}
		, m_draw_grid{false}
		, m_draw_axes{true}
		, m_draw_terrain_nodes{false}
		, m_draw_terrain_wireframe{false}
		, m_visualise_terrain_normals{false}
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
		m_view_properties_buffer.set_data(m_scene_system.get_current_scene_view_info(), 0);

		LOG("[OPENGL] Constructed new OpenGLRenderer instance");
	}

	void OpenGLRenderer::draw(const DeltaTime& delta_time, FBO& target_FBO)
	{
		PERF(OpenGLRendererDraw);

		auto& entities  = m_scene_system.get_current_scene_entities();
		auto& scene     = m_scene_system.get_current_scene();
		auto& view_info = m_scene_system.get_current_scene_view_info();

		m_view_properties_buffer.set_data(view_info, 0);
		m_shadow_mapper.shadow_pass(scene);

		{ // Prepare target_FBO for rendering
			auto clear_colour = Platform::Core::s_theme.background;
			clear_colour.a    = 1.f;
			target_FBO.set_clear_colour(clear_colour); // Platform::Core::s_theme.background);
			target_FBO.clear();
			ASSERT(target_FBO.is_complete(), "Screen framebuffer not complete, have you attached a colour or depth buffer to it?");
		}

		glm::mat4 light_proj_view = glm::identity<glm::mat4>();
		if (m_draw_shadows)
		{
			entities.foreach([&](Component::DirectionalLight& p_light)
			{
				light_proj_view = p_light.get_view_proj(scene.m_bound);
				return;
			});
		}

		if (m_draw_grid)
			m_grid_renderer.draw(target_FBO);

		m_phong_renderer.update_light_data(scene);
		const auto& directional_light_buffer = m_phong_renderer.get_directional_lights_buffer();
		const auto& point_light_buffer       = m_phong_renderer.get_point_lights_buffer();
		const auto& spot_light_buffer        = m_phong_renderer.get_spot_lights_buffer();

		entities.foreach([&](ECS::Entity& p_entity, Component::Transform& p_transform, Component::Mesh& mesh_comp)
		{
			if (mesh_comp.m_mesh)
			{
				Shader* mesh_shader = nullptr;
				DrawCall dc;

				if (entities.has_components<Component::Texture>(p_entity))
				{
					auto& texComponent = entities.get_component<Component::Texture>(p_entity);
					dc.set_SSBO("DirectionalLightsBuffer", directional_light_buffer);
					dc.set_SSBO("PointLightsBuffer",       point_light_buffer);
					dc.set_SSBO("SpotLightsBuffer",        spot_light_buffer);
					dc.set_uniform("shininess",            texComponent.m_shininess);

					if (texComponent.m_diffuse.has_value())
					{
						dc.set_texture("diffuse",  texComponent.m_diffuse->m_GL_texture);
						dc.set_texture("specular", texComponent.m_specular.has_value() ? texComponent.m_specular->m_GL_texture : m_blank_texture->m_GL_texture);

						if (m_draw_shadows)
							mesh_shader = &m_phong_renderer.get_texture_shadow_shader();
						else
							mesh_shader = &m_phong_renderer.get_texture_shader();
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
						if (m_draw_shadows)
							mesh_shader = &m_phong_renderer.get_uniform_colour_shadow_shader();
						else
							mesh_shader = &m_phong_renderer.get_uniform_colour_shader();
					}

					if (m_draw_shadows)
					{
						dc.set_uniform("PCF_bias",        Component::DirectionalLight::PCF_bias);
						dc.set_uniform("light_proj_view", light_proj_view);
						dc.set_texture("shadow_map",      m_shadow_mapper.get_depth_map());
					}
				}
				else
				{
					// Fallback to rendering using default colour and no lighting.
					dc.set_uniform("colour", glm::vec4(0.06f, 0.44f, 0.81f, 1.f));
					mesh_shader = &m_uniform_colour_shader;
				}

				dc.set_UBO("ViewProperties", m_view_properties_buffer);
				dc.set_uniform("model", p_transform.get_model());
				dc.submit(*mesh_shader, mesh_comp.m_mesh->get_VAO(), target_FBO);
			}
		});

		{// Draw terrain
			entities.foreach([&](Component::Terrain& p_terrain)
			{
				if (m_draw_terrain_nodes)
				{
					std::array<glm::vec4, 8> depth_colours = {
						glm::vec4(1.f, 0.f, 0.f, 1.f), // Red
						glm::vec4(0.f, 1.f, 0.f, 1.f), // Green
						glm::vec4(0.f, 0.f, 1.f, 1.f), // Blue
						glm::vec4(1.f, 1.f, 0.f, 1.f), // Yellow
						glm::vec4(1.f, 0.5f, 0.5f, 1.f), // Light Red
						glm::vec4(0.5f, 1.f, 0.5f, 1.f), // Light Green
						glm::vec4(0.5f, 0.5f, 1.f, 1.f), // Light Blue
						glm::vec4(1.f, 0.5f, 1.f, 1.f) // Light Purple
					};
					std::vector<std::pair<Geometry::Depth_t, Geometry::AABB2D>> nodes_to_draw;
					nodes_to_draw.reserve(p_terrain.node_mesh_info.size());
					for (const auto& node : p_terrain.node_mesh_info)
					{
						auto node_bounds_2D = node.first.get_bounds(p_terrain.root_bounds->half_size, p_terrain.root_bounds->center);
						nodes_to_draw.emplace_back(node.first.depth, node_bounds_2D);
					}

					std::sort(nodes_to_draw.begin(), nodes_to_draw.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
					for (const auto& [depth, bounds] : nodes_to_draw)
						DebugRenderer::add(Geometry::AABB(glm::vec3{bounds.min.x, 0.f, bounds.min.y}, glm::vec3{bounds.max.x, 0.f, bounds.max.y}), depth_colours[depth % depth_colours.size()], false);

					return;
				}

				if (p_terrain.empty())
					return;

				DrawCall dc;

				if (m_draw_terrain_wireframe) dc.m_polygon_mode = PolygonMode::Line;

				dc.set_SSBO("DirectionalLightsBuffer", directional_light_buffer);
				dc.set_SSBO("PointLightsBuffer",       point_light_buffer);
				dc.set_SSBO("SpotLightsBuffer",        spot_light_buffer);
				dc.set_UBO("ViewProperties",           m_view_properties_buffer);

				dc.set_uniform("model",                glm::identity<glm::mat4>());
				dc.set_uniform("shininess",            1000000.f); // Force terrain to not be shiny.
				// TODO: calc instead of using amplitude... use the actual height of the terrain.
				dc.set_uniform("min_height", -p_terrain.noise_params.height);
				dc.set_uniform("max_height",  p_terrain.noise_params.height);

				dc.set_uniform("debug_normals", m_visualise_terrain_normals);

				dc.set_texture("grass", p_terrain.m_grass_tex->m_GL_texture);
				dc.set_texture("rock",  p_terrain.m_rock_tex->m_GL_texture);
				dc.set_texture("snow",  p_terrain.m_snow_tex->m_GL_texture);

				dc.submit(m_terrain_shader, p_terrain.get_VAO(), target_FBO);
			});
		}

		m_particle_renderer.update(delta_time, scene, view_info.m_view_position, m_view_properties_buffer, target_FBO);

		OpenGL::DebugRenderer::render(m_scene_system, m_view_properties_buffer, m_phong_renderer.get_point_lights_buffer(), target_FBO);

		if (m_post_processing_options.any_active())
		{
			// Apply post-processing effects by rendering the current target_FBO to a screen-sized quad with the post-processing shader.
			// First blit the current target_FBO to the intermediate post-processing FBO. Then render the post-processing FBO to the screen quad with the post-processing shader.
			if (!m_post_processing_FBO)
				m_post_processing_FBO = FBO{target_FBO.resolution(), true, false, false};
			m_post_processing_FBO->resize(target_FBO.resolution());
			target_FBO.blit_to_fbo(*m_post_processing_FBO, true, false, false, InterpolationFilter::Nearest);

			DrawCall post_process_dc;
			post_process_dc.m_depth_test_enabled    = false;
			post_process_dc.m_cull_face_enabled     = false;
			post_process_dc.m_polygon_mode          = PolygonMode::Fill;
			post_process_dc.m_write_to_depth_buffer = false;
			post_process_dc.set_uniform("invertColours", m_post_processing_options.mInvertColours);
			post_process_dc.set_uniform("grayScale", m_post_processing_options.mGrayScale);
			post_process_dc.set_uniform("sharpen", m_post_processing_options.mSharpen);
			post_process_dc.set_uniform("blur", m_post_processing_options.mBlur);
			post_process_dc.set_uniform("edgeDetection", m_post_processing_options.mEdgeDetection);
			post_process_dc.set_uniform("offset", m_post_processing_options.mKernelOffset);
			post_process_dc.set_texture("screen_texture", m_post_processing_FBO->color_attachment());
			post_process_dc.submit(m_screen_texture_shader, m_screen_quad.get_VAO(), *m_post_processing_FBO);
			m_post_processing_FBO->blit_to_fbo(target_FBO, true, false, false, InterpolationFilter::Nearest);
		}
		if (m_draw_axes)
		{
			DrawCall axes_dc;
			float axis_size = 1.f;
			axes_dc.set_uniform("model", glm::scale(glm::identity<glm::mat4>(), glm::vec3(axis_size)));

			// Copy and modify the view information to create an orthographic projection with no translation
			auto view_prop         = m_scene_system.get_current_scene_view_info();
			view_prop.m_view       = glm::mat4(glm::mat3(view_prop.m_view)); // Remove translation from view matrix.
			view_prop.m_projection = glm::ortho(-axis_size, axis_size, -axis_size, axis_size, -axis_size, axis_size);
			m_view_properties_buffer.set_data(view_prop, 0);
			axes_dc.set_UBO("ViewProperties", m_view_properties_buffer);

			axes_dc.m_depth_test_enabled    = true;
			axes_dc.m_write_to_depth_buffer = true;
			axes_dc.m_cull_face_enabled     = true;

			glm::uvec2 res          = target_FBO.resolution();
			glm::uvec2 axes_size    = glm::uvec2{std::min(res.x, res.y) / 16u};
			glm::uvec2 axes_padding = glm::uvec2{std::min(res.x, res.y) / 64u};
			axes_dc.submit(m_colour_shader, m_axis_mesh.get_VAO(), target_FBO, axes_padding, axes_size);
		}
	}

	void OpenGLRenderer::draw_UI()
	{
		ImGui::Checkbox("Draw shadows",           &m_draw_shadows);
		ImGui::Checkbox("Draw grid",              &m_draw_grid);
		ImGui::Checkbox("Draw axes",              &m_draw_axes);
		ImGui::Checkbox("Draw terrain nodes",     &m_draw_terrain_nodes);
		ImGui::Checkbox("Draw terrain wireframe", &m_draw_terrain_wireframe);
		ImGui::Checkbox("Debug terrain Normals",  &m_visualise_terrain_normals);

		if (ImGui::Button("Reload Shaders"))
			reload_shaders();

		{ ImGui::SeparatorText("Post Processing");
			ImGui::Checkbox("Invert",         &m_post_processing_options.mInvertColours);
			ImGui::Checkbox("Grayscale",      &m_post_processing_options.mGrayScale);
			ImGui::Checkbox("Sharpen",        &m_post_processing_options.mSharpen);
			ImGui::Checkbox("Blur",           &m_post_processing_options.mBlur);
			ImGui::Checkbox("Edge detection", &m_post_processing_options.mEdgeDetection);

			const bool post_processing_active = m_post_processing_options.mInvertColours
				|| m_post_processing_options.mGrayScale || m_post_processing_options.mSharpen
				|| m_post_processing_options.mBlur      || m_post_processing_options.mEdgeDetection;

			if (!post_processing_active) ImGui::BeginDisabled();
				ImGui::SliderFloat("Kernel offset", &m_post_processing_options.mKernelOffset, -1.f, 1.f);
			if (!post_processing_active) ImGui::EndDisabled();
		}
	}

	void OpenGLRenderer::reset_debug_options()
	{
		m_draw_shadows              = true;
		m_draw_grid                 = true;
		m_draw_terrain_nodes        = false;
		m_draw_terrain_wireframe    = false;
		m_visualise_terrain_normals = false;
		m_post_processing_options   = {};
	}
	void OpenGLRenderer::reload_shaders()
	{
		m_uniform_colour_shader.reload();
		m_colour_shader.reload();
		m_texture_shader.reload();
		m_screen_texture_shader.reload();
		m_sky_box_shader.reload();
		m_terrain_shader.reload();
		m_particle_renderer.reload_shaders();
		m_phong_renderer.reload_shaders();
		m_grid_renderer.reload_shaders();
		m_shadow_mapper.reload_shaders();
	}
} // namespace OpenGL