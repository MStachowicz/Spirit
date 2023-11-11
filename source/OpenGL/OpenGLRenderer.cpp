#include "OpenGLRenderer.hpp"
#include "OpenGL/DebugRenderer.hpp"
#include "OpenGL/DrawCall.hpp"
// ECS
#include "Storage.hpp"

// COMPONENTS
#include "Component/Lights.hpp"
#include "Component/Texture.hpp"
#include "Component/Collider.hpp"
#include "Component/Camera.hpp"
#include "Component/Mesh.hpp"
#include "Component/Transform.hpp"
#include "Component/Terrain.hpp"

// SYSTEMS
#include "System/MeshSystem.hpp"
#include "System/TextureSystem.hpp"
#include "System/SceneSystem.hpp"


// UTILITY
#include "Utility/Logger.hpp"
#include "Utility/Utility.hpp"
#include "Utility/Config.hpp"

// PLATFORM
#include "Core.hpp"
#include "Window.hpp"

//GLM
#include "glm/ext/matrix_transform.hpp" // perspective, translate, rotate
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/string_cast.hpp"

// STD
#include <algorithm>

#include "glad/gl.h"

namespace OpenGL
{
	OpenGLRenderer::OpenGLRenderer(Platform::Window& p_window, System::SceneSystem& pSceneSystem, System::MeshSystem& pMeshSystem, System::TextureSystem& pTextureSystem) noexcept
		: m_window{p_window}
		, mScreenFramebuffer{}
		, mSceneSystem{pSceneSystem}
		, mMeshSystem{pMeshSystem}
		, mPostProcessingOptions{}
		, mUniformColourShader{"uniformColour"}
		, m_colour_shader{"colour"}
		, mTextureShader{"texture1"}
		, mScreenTextureShader{"screenTexture"}
		, mSkyBoxShader{"skybox"}
		, m_phong_renderer{}
		, m_particle_renderer{}
		, m_light_position_renderer{}
		, m_shadow_mapper{p_window}
		, m_missing_texture{pTextureSystem.mTextureManager.insert(Data::Texture{Config::Texture_Directory / "missing.png"})}
		, m_blank_texture{pTextureSystem.mTextureManager.insert(Data::Texture{Config::Texture_Directory / "black.jpg"})}
		, mViewInformation{}
	{
		const auto windowSize = m_window.size();
		mScreenFramebuffer.attachColourBuffer(windowSize.x, windowSize.y);
		mScreenFramebuffer.attachDepthBuffer(windowSize.x, windowSize.y);
		set_viewport(0, 0, windowSize.x, windowSize.y);

		LOG("Constructed new OpenGLRenderer instance");
	}

	void OpenGLRenderer::draw(const Data::Model& pModel)
	{
		draw(pModel.mCompositeMesh);
	}
	void OpenGLRenderer::draw(const Data::CompositeMesh& pComposite)
	{
		for (const auto& mesh : pComposite.mMeshes)
			draw(mesh);

		// Recursively draw all the child composites.
		for (const auto& childComposite : pComposite.mChildMeshes)
			draw(childComposite);
	}
	void OpenGLRenderer::draw(const Data::Mesh& pMesh)
	{
		const auto& GLMeshData = pMesh.mGLData;
		GLMeshData.mVAO.bind();

		if (GLMeshData.mEBO.has_value()) // EBO available means drawing with indices.
			draw_elements(PrimitiveMode::Triangles, GLMeshData.mDrawSize);
		else
			draw_arrays(PrimitiveMode::Triangles, 0, GLMeshData.mDrawSize);
	}

	void OpenGLRenderer::start_frame()
	{
		{ // Set global shader uniforms.
			mSceneSystem.getCurrentScene().foreach([this](Component::Camera& p_camera, Component::Transform& p_transform)
			{
				if (p_camera.m_primary)
				{
					mViewInformation.mViewPosition = p_transform.mPosition;
					mViewInformation.mView         = p_camera.view(p_transform.mPosition);// glm::lookAt(p_transform.mPosition, p_transform.mPosition + p_transform.mDirection, camera_up);
					mViewInformation.mProjection   = glm::perspective(glm::radians(p_camera.m_FOV), m_window.aspect_ratio(), p_camera.m_near, p_camera.m_far);

					Shader::set_block_uniform("ViewProperties.view", mViewInformation.mView);
					Shader::set_block_uniform("ViewProperties.projection", mViewInformation.mProjection);
				}
			});
		}

		FBO::unbind();

		m_shadow_mapper.shadow_pass(mSceneSystem.m_scene);

		{ // Prepare mScreenFramebuffer for rendering
			const auto window_size = m_window.size();
			mScreenFramebuffer.resize(window_size.x, window_size.y);
			set_viewport(0, 0, window_size.x, window_size.y);
			mScreenFramebuffer.bind();
			mScreenFramebuffer.clearBuffers();
			ASSERT(mScreenFramebuffer.isComplete(), "Screen framebuffer not complete, have you attached a colour or depth buffer to it?");
		}
	}

	void OpenGLRenderer::draw(const DeltaTime& delta_time)
	{
		m_phong_renderer.update_light_data(mSceneSystem.m_scene, m_shadow_mapper.get_depth_map());

		auto& scene = mSceneSystem.getCurrentScene();
		scene.foreach([&](ECS::Entity& pEntity, Component::Transform& p_transform, Component::Mesh& p_mesh)
		{
			if (scene.hasComponents<Component::Texture>(pEntity))
			{
				auto& texComponent = scene.getComponent<Component::Texture>(pEntity);
				m_phong_renderer.set_draw_data(
					mViewInformation.mViewPosition,
					p_transform.mModel,
					texComponent.mDiffuse.has_value()  ? texComponent.mDiffuse->m_GL_texture  : m_missing_texture->m_GL_texture,
					texComponent.mSpecular.has_value() ? texComponent.mSpecular->m_GL_texture : m_blank_texture->m_GL_texture,
					texComponent.m_shininess);
			}
			else
			{
				mUniformColourShader.use();
				mUniformColourShader.set_uniform("model", p_transform.mModel);
				mUniformColourShader.set_uniform("colour", glm::vec3(0.06f, 0.44f, 0.81f));
			}

			draw(*p_mesh.mModel);
		});

		{// Draw terrain
			scene.foreach([&](Component::Terrain& p_terrain)
			{
				m_phong_renderer.set_draw_data(
					mViewInformation.mViewPosition,
					glm::translate(glm::identity<glm::mat4>(), p_terrain.position),
					p_terrain.texture.has_value() ? p_terrain.texture->m_GL_texture : m_missing_texture->m_GL_texture,
					m_blank_texture->m_GL_texture,
					64.f);

				p_terrain.mesh.draw();
			});
		}

		m_particle_renderer.update(delta_time, mSceneSystem.m_scene, mViewInformation.mViewPosition);
	}

	void OpenGLRenderer::end_frame()
	{
		{ // Draw the colour output to the from mScreenFramebuffer texture to the default FBO
			// Unbind after completing draw to ensure all subsequent actions apply to the default FBO and not mScreenFrameBuffer.
			// Disable depth testing to not cull the screen quad the screen texture will be applied onto.
			FBO::unbind();
			set_depth_test(false);
			set_cull_face(false);
			set_polygon_mode(PolygonMode::Fill);
			glClear(GL_COLOR_BUFFER_BIT);

			mScreenTextureShader.use();
			{ // PostProcessing setters
				mScreenTextureShader.set_uniform("invertColours", mPostProcessingOptions.mInvertColours);
				mScreenTextureShader.set_uniform("grayScale", mPostProcessingOptions.mGrayScale);
				mScreenTextureShader.set_uniform("sharpen", mPostProcessingOptions.mSharpen);
				mScreenTextureShader.set_uniform("blur", mPostProcessingOptions.mBlur);
				mScreenTextureShader.set_uniform("edgeDetection", mPostProcessingOptions.mEdgeDetection);
				mScreenTextureShader.set_uniform("offset", mPostProcessingOptions.mKernelOffset);
			}

			active_texture(0);
			mScreenFramebuffer.bindColourTexture();

			draw(*mMeshSystem.mPlanePrimitive);
		}
	}
} // namespace OpenGL