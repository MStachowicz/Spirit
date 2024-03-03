#pragma once

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
	class MeshSystem;
	class TextureSystem;
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
		FBO m_screen_framebuffer;
		System::SceneSystem& m_scene_system;
		System::MeshSystem& m_mesh_system;

		Shader m_uniform_colour_shader;
		Shader m_colour_shader;
		Shader m_texture_shader;
		Shader m_screen_texture_shader;
		Shader m_sky_box_shader;

		PhongRenderer m_phong_renderer;
		ParticleRenderer m_particle_renderer;
		ShadowMapper m_shadow_mapper;
		TextureRef m_missing_texture;
		TextureRef m_blank_texture;
		Data::Mesh m_screen_quad;

	public:
		PostProcessingOptions m_post_processing_options;

		// OpenGLRenderer reads and renders the current state of pStorage when draw() is called.
		OpenGLRenderer(Platform::Window& p_window, System::SceneSystem& p_scene_system, System::MeshSystem& p_mesh_system, System::TextureSystem& p_texture_system) noexcept;

		void start_frame();
		void end_frame();
		// Draw the current state of the ECS.
		void draw(const DeltaTime& delta_time);
	};
} // namespace OpenGL