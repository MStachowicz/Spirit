#include "Editor.hpp"

// ECS
#include "Storage.hpp"

// SYSTEM
#include "CollisionSystem.hpp"
#include "MeshSystem.hpp"
#include "SceneSystem.hpp"
#include "TextureSystem.hpp"

// COMPONENT
#include "Camera.hpp"
#include "Collider.hpp"
#include "DirectionalLight.hpp"
#include "Label.hpp"
#include "Mesh.hpp"
#include "PointLight.hpp"
#include "RigidBody.hpp"
#include "SpotLight.hpp"
#include "Texture.hpp"
#include "Transform.hpp"

// GEOMETRY
#include "Geometry.hpp"

// OPENGL
#include "OpenGLRenderer.hpp"
#include "DebugRenderer.hpp"

// PLATFORM
#include "Core.hpp"
#include "Input.hpp"
#include "Window.hpp"

// UTILITY
#include "Logger.hpp"
#include "Utility.hpp"

// GLM
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

// IMGUI
#include "imgui.h"

// STD
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
                        auto entitiesUnderMouse = mCollisionSystem.getEntitiesAlongRay(cursorRay);

                        if (!entitiesUnderMouse.empty())
                        {
                            std::sort(entitiesUnderMouse.begin(), entitiesUnderMouse.end(), [](const auto& left, const auto& right) { return left.second < right.second; });
                            auto entityCollided = entitiesUnderMouse.front().first;

                            mSelectedEntities.push_back(entityCollided);
                            LOG("[EDITOR] Entity{} has been selected", entityCollided.ID);
                        }

                        const auto mouseRayCylinder = Geometry::Cylinder(mSceneSystem.getPrimaryCamera()->get_position(), mSceneSystem.getPrimaryCamera()->get_position() + (cursorRay.m_direction * 1000.f), 0.02f);
                        mOpenGLRenderer.mDebugOptions.mCylinders.push_back(mouseRayCylinder);
                    }
                    break;
                }
                case Platform::MouseButton::Middle:
                {
                    mOpenGLRenderer.mDebugOptions.mCylinders.clear();
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

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("View"))
            {
                ImGui::MenuItem("Entity hierarchy", NULL, &mWindowsToDisplay.Entity);
                ImGui::MenuItem("Console", NULL, &mWindowsToDisplay.Console);

                if (ImGui::BeginMenu("Debug"))
                {
                    ImGui::MenuItem("Performance", NULL, &mWindowsToDisplay.Performance);
                    ImGui::MenuItem("FPS Timer", NULL, &mWindowsToDisplay.FPSTimer);
                    ImGui::MenuItem("Graphics", NULL, &mWindowsToDisplay.Graphics);
                    ImGui::MenuItem("Physics", NULL, &mWindowsToDisplay.Physics);

                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("ImGui"))
                {
                    ImGui::MenuItem("Demo", NULL, &mWindowsToDisplay.ImGuiDemo);
                    ImGui::MenuItem("Metrics/Debugger", NULL, &mWindowsToDisplay.ImGuiMetrics);
                    ImGui::MenuItem("Stack", NULL, &mWindowsToDisplay.ImGuiStack);
                    ImGui::MenuItem("About", NULL, &mWindowsToDisplay.ImGuiAbout);
                    ImGui::MenuItem("Style Editor", NULL, &mWindowsToDisplay.ImGuiStyleEditor);

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
        if (mWindowsToDisplay.Entity)           drawEntityTreeWindow();
        if (mWindowsToDisplay.Console)          drawConsoleWindow();
        if (mWindowsToDisplay.Performance)      drawPerformanceWindow();
        if (mWindowsToDisplay.Graphics)         drawGraphicsWindow();
        if (mWindowsToDisplay.Physics)          drawPhysicsWindow();
        if (mWindowsToDisplay.ImGuiDemo)        ImGui::ShowDemoWindow(&mWindowsToDisplay.ImGuiDemo);
        if (mWindowsToDisplay.ImGuiMetrics)     ImGui::ShowMetricsWindow(&mWindowsToDisplay.ImGuiMetrics);
        if (mWindowsToDisplay.ImGuiStack)       ImGui::ShowStackToolWindow(&mWindowsToDisplay.ImGuiStack);
        if (mWindowsToDisplay.ImGuiAbout)       ImGui::ShowAboutWindow(&mWindowsToDisplay.ImGuiAbout);
        if (mWindowsToDisplay.ImGuiStyleEditor)
        {
            ImGui::Begin("Dear ImGui Style Editor", &mWindowsToDisplay.ImGuiStyleEditor);
            ImGui::ShowStyleEditor();
            ImGui::End();
        }

        mDrawCount++;
    }

    void Editor::drawEntityTreeWindow()
    {
        if (ImGui::Begin("Entities", &mWindowsToDisplay.Entity))
        {
            auto& availableTextures = mTextureSystem.mAvailableTextures;
            std::vector<std::string> availableTextureNames;
            for (const auto& path : availableTextures)
                availableTextureNames.push_back(path.stem().string());

            auto& availableModels = mMeshSystem.mAvailableModels;
            std::vector<std::string> availableModelNames;
            for (const auto& path : availableModels)
                availableModelNames.push_back(path.stem().string());

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
                        scene.getComponentMutable<Component::Collider&>(pEntity).DrawImGui();
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
                    if (scene.hasComponents<Component::Mesh>(pEntity))
                    {
                        auto& mesh = scene.getComponentMutable<Component::Mesh>(pEntity);
                        if (ImGui::TreeNode("Mesh"))
                        {
                            auto current = mesh.mModel->mFilePath.stem().string();
                            static size_t selected;
                            if (ImGui::ComboContainer("Mesh", current.c_str(), availableModelNames, selected))
                                mesh.mModel = mMeshSystem.getModel(availableModels[selected]);

                            ImGui::TreePop();
                        }
                    }
                    if (scene.hasComponents<Component::Texture>(pEntity))
                    {
                        if (ImGui::TreeNode("Texture"))
                        {
                            auto& textureComponent = scene.getComponentMutable<Component::Texture&>(pEntity);
                            const std::string currentDiffuse  = textureComponent.mDiffuse.has_value() ? textureComponent.mDiffuse.value()->m_image_ref->name() : "None";
                            const std::string currentSpecular = textureComponent.mSpecular.has_value() ? textureComponent.mSpecular.value()->m_image_ref->name() : "None";

                            static size_t selected;
                            if (ImGui::ComboContainer("Diffuse Texture", currentDiffuse.c_str(), availableTextureNames, selected))
                                textureComponent.mDiffuse = mTextureSystem.getTexture(availableTextures[selected]);
                            if (ImGui::ComboContainer("Specular Texture", currentSpecular.c_str(), availableTextureNames, selected))
                                textureComponent.mSpecular = mTextureSystem.getTexture(availableTextures[selected]);

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

    void Editor::drawConsoleWindow()
    {
        m_console.draw("Console", &mWindowsToDisplay.Console);
    }

    void Editor::drawGraphicsWindow()
    {
        if (ImGui::Begin("Graphics", &mWindowsToDisplay.Graphics))
        {
            ImGui::Text("Window size", m_window.size());
            ImGui::Text("Aspect ratio", m_window.aspect_ratio());
            bool VSync = m_window.get_VSync();
            if (ImGui::Checkbox("VSync", &VSync)) m_window.set_VSync(VSync);
            ImGui::Text("View Position", mOpenGLRenderer.mViewInformation.mViewPosition);
            ImGui::SliderFloat("Field of view", &mOpenGLRenderer.mViewInformation.mFOV, 1.f, 120.f);
            ImGui::SliderFloat("Z near plane", &mOpenGLRenderer.mViewInformation.mZNearPlane, 0.001f, 15.f);
            ImGui::SliderFloat("Z far plane", &mOpenGLRenderer.mViewInformation.mZFarPlane, 15.f, 300.f);
            ImGui::Separator();

            if (ImGui::TreeNode("PostProcessing"))
            {
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

                ImGui::TreePop();
            }
            ImGui::Separator();

            ImGui::Checkbox("Force clear colour", &mOpenGLRenderer.mDebugOptions.mForceClearColour);
            if (!mOpenGLRenderer.mDebugOptions.mForceClearColour) ImGui::BeginDisabled();
            {
                ImGui::ColorEdit4("Window clear colour", glm::value_ptr(mOpenGLRenderer.mDebugOptions.mClearColour) );
            }
            if (!mOpenGLRenderer.mDebugOptions.mForceClearColour) ImGui::EndDisabled();

            ImGui::Checkbox("Show light positions", &mOpenGLRenderer.mDebugOptions.mShowLightPositions);
            ImGui::Checkbox("Visualise normals", &mOpenGLRenderer.mDebugOptions.mVisualiseNormals);

            { // Depth options
                ImGui::Checkbox("Force depth test type", &mOpenGLRenderer.mDebugOptions.mForceDepthTestType);

                if (!mOpenGLRenderer.mDebugOptions.mForceDepthTestType) ImGui::BeginDisabled();
                {
                    const std::vector<std::pair<OpenGL::DepthTestType, const char*>> depthTestOptions =
                    {
                        { OpenGL::DepthTestType::Always, "Always" },
                        { OpenGL::DepthTestType::Never, "Never" },
                        { OpenGL::DepthTestType::Less, "Less" },
                        { OpenGL::DepthTestType::Equal, "Equal" },
                        { OpenGL::DepthTestType::NotEqual, "NotEqual" },
                        { OpenGL::DepthTestType::Greater, "Greater" },
                        { OpenGL::DepthTestType::LessEqual, "LessEqual" },
                        { OpenGL::DepthTestType::GreaterEqual, "GreaterEqual" }
                    };
                    ImGui::ComboContainer("Forced depth test type", mOpenGLRenderer.mDebugOptions.mForcedDepthTestType, depthTestOptions );
                }
                if (!mOpenGLRenderer.mDebugOptions.mForceDepthTestType) ImGui::EndDisabled();
            }
            { // Blending options
                ImGui::Checkbox("Force blend type", &mOpenGLRenderer.mDebugOptions.mForceBlendType);

                const std::vector<std::pair<OpenGL::BlendFactorType, const char*>> blendOptions =
                {
                    { OpenGL::BlendFactorType::Zero, "Zero" },
                    { OpenGL::BlendFactorType::One, "One" },
                    { OpenGL::BlendFactorType::SourceColour, "SourceColour" },
                    { OpenGL::BlendFactorType::OneMinusSourceColour, "OneMinusSourceColour" },
                    { OpenGL::BlendFactorType::DestinationColour, "DestinationColour" },
                    { OpenGL::BlendFactorType::OneMinusDestinationColour, "OneMinusDestinationColour" },
                    { OpenGL::BlendFactorType::SourceAlpha, "SourceAlpha" },
                    { OpenGL::BlendFactorType::OneMinusSourceAlpha, "OneMinusSourceAlpha" },
                    { OpenGL::BlendFactorType::DestinationAlpha, "DestinationAlpha" },
                    { OpenGL::BlendFactorType::OneMinusDestinationAlpha, "OneMinusDestinationAlpha" },
                    { OpenGL::BlendFactorType::ConstantColour, "ConstantColour" },
                    { OpenGL::BlendFactorType::OneMinusConstantColour, "OneMinusConstantColour" },
                    { OpenGL::BlendFactorType::ConstantAlpha, "ConstantAlpha" },
                    { OpenGL::BlendFactorType::OneMinusConstantAlpha, "OneMinusConstantAlpha" }
                };
                if (!mOpenGLRenderer.mDebugOptions.mForceBlendType) ImGui::BeginDisabled();
                {
                    ImGui::ComboContainer("Source", mOpenGLRenderer.mDebugOptions.mForcedSourceBlendType, blendOptions );
                    ImGui::ComboContainer("Destination", mOpenGLRenderer.mDebugOptions.mForcedDestinationBlendType, blendOptions );
                }
                if (!mOpenGLRenderer.mDebugOptions.mForceBlendType) ImGui::EndDisabled();
            }
            { // Cull face options
                const std::vector<std::pair<OpenGL::CullFacesType, const char*>> cullFaceOptions =
                {
                    { OpenGL::CullFacesType::Back, "Back" },
                    { OpenGL::CullFacesType::Front, "Front" },
                    { OpenGL::CullFacesType::FrontAndBack, "FrontAndBack" }
                };

                ImGui::Checkbox("Force cull face type", &mOpenGLRenderer.mDebugOptions.mForceCullFacesType);
                if (!mOpenGLRenderer.mDebugOptions.mForceCullFacesType) ImGui::BeginDisabled();
                {
                    ImGui::ComboContainer("Forced cull faces type", mOpenGLRenderer.mDebugOptions.mForcedCullFacesType, cullFaceOptions );
                }
                if (!mOpenGLRenderer.mDebugOptions.mForceCullFacesType) ImGui::EndDisabled();

                const std::vector<std::pair<OpenGL::FrontFaceOrientation, const char*>> frontFaceOrientationOptions =
                {
                    { OpenGL::FrontFaceOrientation::Clockwise, "Clockwise" },
                    { OpenGL::FrontFaceOrientation::CounterClockwise, "CounterClockwise" }
                };

                ImGui::Checkbox("Force front face type", &mOpenGLRenderer.mDebugOptions.mForceFrontFaceOrientationType);
                if (!mOpenGLRenderer.mDebugOptions.mForceFrontFaceOrientationType) ImGui::BeginDisabled();
                {
                    ImGui::ComboContainer("Forced front face type", mOpenGLRenderer.mDebugOptions.mForcedFrontFaceOrientationType, frontFaceOrientationOptions );
                }
                if (!mOpenGLRenderer.mDebugOptions.mForceFrontFaceOrientationType) ImGui::EndDisabled();
            }

            if (ImGui::Button("Reset"))
            {
                mOpenGLRenderer.mDebugOptions.mShowLightPositions = false;
                mOpenGLRenderer.mDebugOptions.mVisualiseNormals = false;
                mOpenGLRenderer.mDebugOptions.mForceClearColour = false;
                mOpenGLRenderer.mDebugOptions.mClearColour = glm::vec4(0.f, 0.f, 0.f, 0.f);
                mOpenGLRenderer.mDebugOptions.mForceDepthTestType = false;
                mOpenGLRenderer.mDebugOptions.mForcedDepthTestType = OpenGL::DepthTestType::Less;
                mOpenGLRenderer.mDebugOptions.mForceBlendType = false;
                mOpenGLRenderer.mDebugOptions.mForcedSourceBlendType = OpenGL::BlendFactorType::SourceAlpha;
                mOpenGLRenderer.mDebugOptions.mForcedDestinationBlendType = OpenGL::BlendFactorType::OneMinusSourceAlpha;
                mOpenGLRenderer.mDebugOptions.mForceCullFacesType = false;
                mOpenGLRenderer.mDebugOptions.mForcedCullFacesType = OpenGL::CullFacesType::Back;
                mOpenGLRenderer.mDebugOptions.mForceFrontFaceOrientationType = false;
                mOpenGLRenderer.mDebugOptions.mForcedFrontFaceOrientationType = OpenGL::FrontFaceOrientation::CounterClockwise;
            }

            // TODO: Draw depth buffer in a box #45
            //mBufferDrawType = static_cast<BufferDrawType>(i);
        }
        ImGui::End();
    }

    void Editor::drawPerformanceWindow()
    {
        if (ImGui::Begin("Performance", &mWindowsToDisplay.Performance))
        {
        }
        ImGui::End();
    }

    void Editor::drawPhysicsWindow()
    {
        if (ImGui::Begin("Physics", &mWindowsToDisplay.Physics))
        {
            ImGui::Checkbox("Show collision geometry", &mOpenGLRenderer.mDebugOptions.mShowCollisionGeometry);
            ImGui::Checkbox("Show orientations", &mOpenGLRenderer.mDebugOptions.mShowOrientations);
            ImGui::Checkbox("Show bounding boxes", &mOpenGLRenderer.mDebugOptions.mShowBoundingBoxes);

            if (!mOpenGLRenderer.mDebugOptions.mShowBoundingBoxes) ImGui::BeginDisabled();
            ImGui::Checkbox("Fill bounding boxes ", &mOpenGLRenderer.mDebugOptions.mFillBoundingBoxes);
            if (!mOpenGLRenderer.mDebugOptions.mShowBoundingBoxes) ImGui::EndDisabled();
        }
        ImGui::End();
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