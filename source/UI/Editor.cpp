#include "Editor.hpp"

#include "ECS/Storage.hpp"
#include "Component/Camera.hpp"
#include "Component/Collider.hpp"
#include "Component/Label.hpp"
#include "Component/Lights.hpp"
#include "Component/Mesh.hpp"
#include "Component/RigidBody.hpp"
#include "Component/Terrain.hpp"
#include "Component/Texture.hpp"
#include "Component/Transform.hpp"
#include "System/CollisionSystem.hpp"
#include "System/MeshSystem.hpp"
#include "System/SceneSystem.hpp"
#include "System/TextureSystem.hpp"

#include "OpenGL/DebugRenderer.hpp"
#include "OpenGL/OpenGLRenderer.hpp"
#include "Platform/Core.hpp"
#include "Platform/Input.hpp"
#include "Platform/Window.hpp"
#include "Utility/Logger.hpp"
#include "Utility/Utility.hpp"
#include "imgui.h"

#include "glm/glm.hpp"

#include <format>

namespace UI
{
	Editor::Editor(Platform::Input& p_input, Platform::Window& p_window, System::TextureSystem& pTextureSystem, System::MeshSystem& pMeshSystem, System::SceneSystem& pSceneSystem, System::CollisionSystem& pCollisionSystem, OpenGL::OpenGLRenderer& pOpenGLRenderer)
		: m_input{p_input}
		, m_window{p_window}
		, mTextureSystem{pTextureSystem}
		, mMeshSystem{pMeshSystem}
		, mSceneSystem{pSceneSystem}
		, mCollisionSystem{pCollisionSystem}
		, mOpenGLRenderer{pOpenGLRenderer}
		, m_click_rays{}
		, mSelectedEntities{}
		, m_console{}
		, mWindowsToDisplay{}
		, mDrawCount{0}
		, m_time_to_average_over{std::chrono::seconds(1)}
		, m_duration_between_draws{}
	{
		m_input.m_mouse_event.subscribe(this, &Editor::on_mouse_event);
		m_input.m_key_event.subscribe(this,   &Editor::on_key_event);

		initialiseStyling();
	}

	void Editor::on_mouse_event(Platform::MouseButton p_button, Platform::Action p_action)
	{
		if (p_button == Platform::MouseButton::Right && p_action == Platform::Action::Press)
		{
			if (m_input.cursor_captured())
				m_input.set_cursor_mode(Platform::CursorMode::Normal);
			else if (!m_input.cursor_over_UI()) // We are editing. If we click on non-UI re-capture mouse
				m_input.set_cursor_mode(Platform::CursorMode::Captured);
		}
		if (m_input.cursor_captured())
			return;

		if (!m_input.cursor_over_UI())
		{
			switch (p_button)
			{
				case Platform::MouseButton::Left:
				{
					if (p_action == Platform::Action::Press)
					{
						const auto& view_info = mOpenGLRenderer.mViewInformation;
						auto cursorRay = Utility::get_cursor_ray(m_input.cursor_position(), m_window.size(), view_info.mViewPosition, view_info.mProjection, view_info.mView);
						m_click_rays.emplace_back(cursorRay);
						auto entitiesUnderMouse = mCollisionSystem.getEntitiesAlongRay(cursorRay);

						if (!entitiesUnderMouse.empty())
						{
							std::sort(entitiesUnderMouse.begin(), entitiesUnderMouse.end(), [](const auto& left, const auto& right) { return left.second < right.second; });
							auto entityCollided = entitiesUnderMouse.front().first;

							mSelectedEntities.push_back(entityCollided);
							LOG("[EDITOR] Entity{} has been selected", entityCollided.ID);
						}
					}
					break;
				}
				case Platform::MouseButton::Middle:
				{
					m_click_rays.clear();
					break;
				}
				case Platform::MouseButton::Right: break;
				default: break;
			}
		}
	}
	void Editor::on_key_event(Platform::Key p_key, Platform::Action p_action)
	{}

	void Editor::draw(const DeltaTime& p_duration_since_last_draw)
	{
		m_duration_between_draws.push_back(p_duration_since_last_draw);

		for (const auto& ray : m_click_rays)
			OpenGL::DebugRenderer::add(ray);

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("Entity hierarchy", NULL, &mWindowsToDisplay.Entity);
				ImGui::MenuItem("Console",          NULL, &mWindowsToDisplay.Console);

				if (ImGui::BeginMenu("Debug"))
				{
					ImGui::MenuItem("Debug options", NULL, &mWindowsToDisplay.Debug);
					ImGui::MenuItem("FPS Timer",     NULL, &mWindowsToDisplay.FPSTimer);
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("ImGui"))
				{
					ImGui::MenuItem("Demo",             NULL, &mWindowsToDisplay.ImGuiDemo);
					ImGui::MenuItem("Metrics/Debugger", NULL, &mWindowsToDisplay.ImGuiMetrics);
					ImGui::MenuItem("Stack",            NULL, &mWindowsToDisplay.ImGuiStack);
					ImGui::MenuItem("About",            NULL, &mWindowsToDisplay.ImGuiAbout);
					ImGui::MenuItem("Style Editor",     NULL, &mWindowsToDisplay.ImGuiStyleEditor);

					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			if (mWindowsToDisplay.FPSTimer)
			{
				auto fps = get_fps(m_duration_between_draws, m_time_to_average_over);
				std::string fps_str = std::format("FPS: {:.1f}", fps);
				ImGui::SameLine((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(fps_str.c_str()).x - ImGui::GetStyle().ItemSpacing.x) / 2.f);

				glm::vec4 colour;
				if      (fps > 60.f) colour = glm::vec4(0.f, 255.f, 0.f, 255.f);
				else if (fps > 30.f) colour = glm::vec4(255.f, 255.f, 0.f, 255.f);
				else                 colour = glm::vec4(255.f, 0.f, 0.f, 255.f);

				ImGui::TextColored(colour, fps_str.c_str());
			}
			ImGui::EndMenuBar();
		}
		if (mWindowsToDisplay.Entity)        draw_entity_tree_window();
		if (mWindowsToDisplay.Console)       draw_console_window();
		draw_debug_window();
		if (mWindowsToDisplay.ImGuiDemo)     ImGui::ShowDemoWindow(&mWindowsToDisplay.ImGuiDemo);
		if (mWindowsToDisplay.ImGuiMetrics)  ImGui::ShowMetricsWindow(&mWindowsToDisplay.ImGuiMetrics);
		if (mWindowsToDisplay.ImGuiStack)    ImGui::ShowStackToolWindow(&mWindowsToDisplay.ImGuiStack);
		if (mWindowsToDisplay.ImGuiAbout)    ImGui::ShowAboutWindow(&mWindowsToDisplay.ImGuiAbout);
		if (mWindowsToDisplay.ImGuiStyleEditor)
		{
			ImGui::Begin("Dear ImGui Style Editor", &mWindowsToDisplay.ImGuiStyleEditor);
			ImGui::ShowStyleEditor();
			ImGui::End();
		}

		mDrawCount++;
	}
	void Editor::draw_entity_tree_window()
	{
		if (ImGui::Begin("Entities", &mWindowsToDisplay.Entity))
		{
			auto& availableTextures = mTextureSystem.mAvailableTextures;
			std::vector<std::string> availableTextureNames;
			for (const auto& path : availableTextures)
				availableTextureNames.push_back(path.stem().string());

			auto& scene = mSceneSystem.getCurrentScene();
			scene.foreachEntity([&](ECS::Entity& pEntity)
			{
				std::string title = "Entity " + std::to_string(pEntity.ID);
				if (scene.hasComponents<Component::Label>(pEntity))
				{
					auto label = scene.getComponentMutable<Component::Label&>(pEntity);
					title = label.mName;
				}

				if (ImGui::TreeNode(title.c_str()))
				{
					if (scene.hasComponents<Component::Transform>(pEntity))
						scene.getComponentMutable<Component::Transform&>(pEntity).DrawImGui();
					if (scene.hasComponents<Component::Collider>(pEntity))
						scene.getComponentMutable<Component::Collider&>(pEntity).draw_UI();
					if (scene.hasComponents<Component::RigidBody>(pEntity))
						scene.getComponentMutable<Component::RigidBody&>(pEntity).DrawImGui();
					if (scene.hasComponents<Component::DirectionalLight>(pEntity))
						scene.getComponentMutable<Component::DirectionalLight&>(pEntity).DrawImGui();
					if (scene.hasComponents<Component::SpotLight>(pEntity))
						scene.getComponentMutable<Component::SpotLight&>(pEntity).DrawImGui();
					if (scene.hasComponents<Component::PointLight>(pEntity))
						scene.getComponentMutable<Component::PointLight&>(pEntity).DrawImGui();
					if (scene.hasComponents<Component::Camera>(pEntity))
						scene.getComponentMutable<Component::Camera>(pEntity).draw_UI();
					if (scene.hasComponents<Component::ParticleEmitter>(pEntity))
						scene.getComponentMutable<Component::ParticleEmitter>(pEntity).draw_UI(mTextureSystem);
					if (scene.hasComponents<Component::Terrain>(pEntity))
						scene.getComponentMutable<Component::Terrain>(pEntity).draw_UI(mTextureSystem);
					if (scene.hasComponents<Component::Mesh>(pEntity))
					{
						auto& mesh = scene.getComponentMutable<Component::Mesh>(pEntity);
						if (ImGui::TreeNode("Mesh"))
						{
							//auto current = mesh.mModel->mFilePath.stem().string();
							//static size_t selected;
							//if (ImGui::ComboContainer("Mesh", current.c_str(), availableModelNames, selected))
							//	mesh.mModel = mMeshSystem.getModel(availableModels[selected]);
							ImGui::TreePop();
						}
					}
					if (scene.hasComponents<Component::Texture>(pEntity))
					{
						if (ImGui::TreeNode("Texture"))
						{
							auto& textureComponent = scene.getComponentMutable<Component::Texture&>(pEntity);
							const std::string currentDiffuse  = textureComponent.mDiffuse ? textureComponent.mDiffuse->m_image_ref->name() : "None";
							const std::string currentSpecular = textureComponent.mSpecular ? textureComponent.mSpecular->m_image_ref->name() : "None";

							static size_t selected;
							if (ImGui::ComboContainer("Diffuse Texture", currentDiffuse.c_str(), availableTextureNames, selected))
								textureComponent.mDiffuse = mTextureSystem.getTexture(availableTextures[selected]);
							if (ImGui::ComboContainer("Specular Texture", currentSpecular.c_str(), availableTextureNames, selected))
								textureComponent.mSpecular = mTextureSystem.getTexture(availableTextures[selected]);
							ImGui::Slider("Shininess", textureComponent.m_shininess, 1.f, 512.f, "%.1f");

							ImGui::TreePop();
						}
					}

					ImGui::SeparatorText("Quick options");
					if (ImGui::Button("Delete entity"))
						scene.deleteEntity(pEntity);

					ImGui::Separator();
					ImGui::TreePop();
				}
			});
		}
		ImGui::End();
	}
	void Editor::draw_console_window()
	{
		m_console.draw("Console", &mWindowsToDisplay.Console);
	}
	void Editor::draw_debug_window()
	{
		if (mWindowsToDisplay.Debug)
		{
			if (ImGui::Begin("Debug options", &mWindowsToDisplay.Debug))
			{
				{ ImGui::SeparatorText("Graphics");
					ImGui::Text("Window size", m_window.size());
					ImGui::Text("Aspect ratio", m_window.aspect_ratio());
					bool VSync = m_window.get_VSync();
					ImGui::Text("View Position", mOpenGLRenderer.mViewInformation.mViewPosition);
					ImGui::Separator();
					ImGui::Checkbox("Show light positions", &OpenGL::DebugRenderer::m_debug_options.m_show_light_positions);
					ImGui::Checkbox("Visualise normals", &OpenGL::DebugRenderer::m_debug_options.m_show_mesh_normals);
					if (ImGui::Checkbox("VSync", &VSync))
						m_window.set_VSync(VSync);
				}

				{ ImGui::SeparatorText("Post Processing");
					ImGui::Checkbox("Invert", &mOpenGLRenderer.mPostProcessingOptions.mInvertColours);
					ImGui::Checkbox("Grayscale", &mOpenGLRenderer.mPostProcessingOptions.mGrayScale);
					ImGui::Checkbox("Sharpen", &mOpenGLRenderer.mPostProcessingOptions.mSharpen);
					ImGui::Checkbox("Blur", &mOpenGLRenderer.mPostProcessingOptions.mBlur);
					ImGui::Checkbox("Edge detection", &mOpenGLRenderer.mPostProcessingOptions.mEdgeDetection);

					const bool isPostProcessingOn = mOpenGLRenderer.mPostProcessingOptions.mInvertColours
						|| mOpenGLRenderer.mPostProcessingOptions.mGrayScale || mOpenGLRenderer.mPostProcessingOptions.mSharpen
						|| mOpenGLRenderer.mPostProcessingOptions.mBlur      || mOpenGLRenderer.mPostProcessingOptions.mEdgeDetection;

					if (!isPostProcessingOn) ImGui::BeginDisabled();
						ImGui::SliderFloat("Kernel offset", &mOpenGLRenderer.mPostProcessingOptions.mKernelOffset, -1.f, 1.f);
					if (!isPostProcessingOn) ImGui::EndDisabled();
				}

				{ImGui::SeparatorText("Physics");
					ImGui::Checkbox("Show orientations", &OpenGL::DebugRenderer::m_debug_options.m_show_orientations);
					ImGui::Checkbox("Show bounding box", &OpenGL::DebugRenderer::m_debug_options.m_show_bounding_box);
					if (!OpenGL::DebugRenderer::m_debug_options.m_show_bounding_box) ImGui::BeginDisabled();
					ImGui::ColorEdit4("Bounding box outline colour", &OpenGL::DebugRenderer::m_debug_options.m_bounding_box_outline_colour[0]);
					ImGui::Checkbox("Fill bounding box", &OpenGL::DebugRenderer::m_debug_options.m_fill_bounding_box);
					if (!OpenGL::DebugRenderer::m_debug_options.m_fill_bounding_box) ImGui::BeginDisabled();
					ImGui::ColorEdit4("Bounding box fill colour", &OpenGL::DebugRenderer::m_debug_options.m_bounding_box_fill_colour[0]);
					if (!OpenGL::DebugRenderer::m_debug_options.m_fill_bounding_box) ImGui::EndDisabled();
					if (!OpenGL::DebugRenderer::m_debug_options.m_show_bounding_box) ImGui::EndDisabled();
					ImGui::Checkbox("Show collision shape", &OpenGL::DebugRenderer::m_debug_options.m_show_collision_shape);
					ImGui::Slider("Position offset factor", OpenGL::DebugRenderer::m_debug_options.m_position_offset_factor, -10.f, 10.f);
					ImGui::Slider("Position offset units", OpenGL::DebugRenderer::m_debug_options.m_position_offset_units, -10.f, 10.f);
				}

				if (ImGui::Button("Reset"))
					OpenGL::DebugRenderer::m_debug_options = OpenGL::DebugRenderer::DebugOptions();
			}
			ImGui::End();
		}
	}

	void Editor::log(const std::string& p_message)
	{
		m_console.add_log({p_message});
	}
	void Editor::log_warning(const std::string& p_message)
	{
		m_console.add_log({p_message, glm::vec3(1.f, 1.f, 0.f)});
	}
	void Editor::log_error(const std::string& p_message)
	{
		m_console.add_log({p_message, glm::vec3(1.f, 0.f, 0.f)});
	}
	void Editor::initialiseStyling()
	{
		ImGui::StyleColorsDark();

		// Round out the UI and make more compact
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowPadding            = ImVec2(4.f, 2.f);
		style.FramePadding             = ImVec2(4.f, 2.f);
		style.CellPadding              = ImVec2(4.f, 0.f);
		style.ItemSpacing              = ImVec2(4.f, 3.f);
		style.ItemInnerSpacing         = ImVec2(4.f, 2.f);
		style.TouchExtraPadding        = ImVec2(0.f, 0.f);
		style.IndentSpacing            = 16.f;
		style.ScrollbarSize            = 10.f;
		style.GrabMinSize              = 10.f;

		style.WindowBorderSize         = 1.f;
		style.ChildBorderSize          = 1.f;
		style.PopupBorderSize          = 1.f;
		style.FrameBorderSize          = 0.f;
		style.TabBorderSize            = 0.f;

		style.WindowRounding           = 4.f;
		style.ChildRounding            = 4.f;
		style.FrameRounding            = 4.f;
		style.PopupRounding            = 4.f;
		style.ScrollbarRounding        = 4.f;
		style.GrabRounding             = 4.f;
		style.LogSliderDeadzone        = 4.f;
		style.TabRounding              = 4.f;

		style.WindowTitleAlign         = ImVec2(0.5f, 0.5f);
		style.WindowMenuButtonPosition = ImGuiDir_Right;
		style.ColorButtonPosition      = ImGuiDir_Right;
		style.ButtonTextAlign          = ImVec2(0.5f, 0.5f);
		style.SelectableTextAlign      = ImVec2(0.5f, 0.5f);
		style.DisplaySafeAreaPadding   = ImVec2(0.f, 0.f);

		auto theme_grey = ImVec4(0.174f, 0.174f, 0.174f, 1.000f);
		style.Colors[ImGuiCol_MenuBarBg] = theme_grey;
	}
} // namespace UI