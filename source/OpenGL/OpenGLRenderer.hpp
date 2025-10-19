#pragma once

#include "GridRenderer.hpp"
#include "ParticleRenderer.hpp"
#include "PhongRenderer.hpp"
#include "Shader.hpp"
#include "ShadowMapper.hpp"
#include "Types.hpp"

#include "Component/Mesh.hpp"
#include "Component/Texture.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace System
{
	class AssetManager;
	class SceneSystem;
}
namespace Platform
{
	class Window;
}
namespace OpenGL
{
	class OpenGLRenderer
	{
		static Data::Mesh make_screen_quad_mesh();

		struct PostProcessingOptions
		{
			bool mInvertColours = false;
			bool mGrayScale     = false;
			bool mSharpen       = false;
			bool mBlur          = false;
			bool mEdgeDetection = false;
			float mKernelOffset = 1.0f / 300.0f;
		};

		Platform::Window& m_window;
		FBO m_screen_framebuffer; // Framebuffer the scene is rendered to. We then render this texture to the screen allowing for post-processing.
		System::AssetManager& m_asset_manager;
		System::SceneSystem& m_scene_system;

		Buffer m_view_properties_buffer; // Contains the view and projection matrices shared across all shaders.
		Shader m_uniform_colour_shader;
		Shader m_colour_shader;
		Shader m_texture_shader;
		Shader m_screen_texture_shader;
		Shader m_sky_box_shader;
		Shader m_terrain_shader;

		PhongRenderer m_phong_renderer;
		ParticleRenderer m_particle_renderer;
		GridRenderer m_grid_renderer;
		ShadowMapper m_shadow_mapper;
		TextureRef m_missing_texture;
		TextureRef m_blank_texture;
		Data::Mesh m_screen_quad;
		Data::Mesh m_axis_mesh;

		PostProcessingOptions m_post_processing_options;
		bool m_draw_shadows;

		public:
		bool m_draw_grid;
		bool m_draw_axes;
		bool m_draw_terrain_nodes;
		bool m_draw_terrain_wireframe;
		bool m_visualise_terrain_normals;

		// OpenGLRenderer reads and renders the current state of pStorage when draw() is called.
		OpenGLRenderer(Platform::Window& p_window, System::AssetManager& p_asset_manager, System::SceneSystem& p_scene_system) noexcept;

		void start_frame();
		void end_frame();
		// Draw the current state of the ECS.
		void draw(const DeltaTime& delta_time);
		void draw_UI();
		void reset_debug_options();
		void reload_shaders();
	};
} // namespace OpenGL