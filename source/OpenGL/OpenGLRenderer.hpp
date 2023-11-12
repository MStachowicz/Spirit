#pragma once

#include "OpenGL/ParticleRenderer.hpp"
#include "OpenGL/PhongRenderer.hpp"
#include "OpenGL/Shader.hpp"
#include "OpenGL/ShadowMapper.hpp"
#include "OpenGL/Types.hpp"

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
		struct ViewInformation
		{
			glm::mat4 mView         = {glm::identity<glm::mat4>()};
			glm::vec3 mViewPosition = {glm::vec3{0.f}};
			glm::mat4 mProjection   = {glm::identity<glm::mat4>()};
		};

		Platform::Window& m_window;
		FBO mScreenFramebuffer;
		System::SceneSystem& mSceneSystem;
		System::MeshSystem& mMeshSystem;

		Shader mUniformColourShader;
		Shader m_colour_shader;
		Shader mTextureShader;
		Shader mScreenTextureShader;
		Shader mSkyBoxShader;

		PhongRenderer m_phong_renderer;
		ParticleRenderer m_particle_renderer;
		ShadowMapper m_shadow_mapper;
		TextureRef m_missing_texture;
		TextureRef m_blank_texture;
		Data::Mesh m_screen_quad;

	public:
		ViewInformation mViewInformation;
		PostProcessingOptions mPostProcessingOptions;

		// OpenGLRenderer reads and renders the current state of pStorage when draw() is called.
		OpenGLRenderer(Platform::Window& p_window, System::SceneSystem& pSceneSystem, System::MeshSystem& pMeshSystem, System::TextureSystem& pTextureSystem) noexcept;

		void start_frame();
		void end_frame();
		// Draw the current state of the ECS.
		void draw(const DeltaTime& delta_time);
	};
} // namespace OpenGL