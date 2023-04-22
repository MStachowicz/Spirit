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

// PLATFORM
#include "Core.hpp"
#include "InputDefinitions.hpp"

// UTILITY
#include "Logger.hpp"

// GLM
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

// IMGUI
#include "imgui.h"

// STD
#include <format>

namespace ImGui
{
    bool ComboContainer(const char* pLabel, const char* pCurrentValue, const std::vector<std::string>& pItems, size_t& outSelectedIndex)
    {
        if (pItems.empty())
            return false;

        bool result = false;
        if (ImGui::BeginCombo(pLabel, pCurrentValue))
        {
            for (size_t i = 0; i < pItems.size(); i++)
            {
                if (ImGui::Selectable(pItems[i].c_str()))
                {
                    outSelectedIndex = i;
                    result = true;
                }
            }
            ImGui::EndCombo();
        }
        return result;
    }

    // Given a pCurrent and a list of pOptions and their labels, creates an ImGui selectable dropdown and assigns the selected option to pCurrent.
    // Returns true if pCurrent has been assigned a new value.
    template <typename Type>
    bool ComboContainer(const char* pLabel, Type& pCurrent, const std::vector<std::pair<Type, const char*>>& pOptions)
    {
        if (pOptions.empty())
            return false;

        const auto it = std::find_if(pOptions.begin(), pOptions.end(), [&pCurrent](const auto& pElement) { return pCurrent == pElement.first; });
        ASSERT(it != pOptions.end(), "pCurrent not found in the list pOptions, pOptions should be a complete list of all types of Type.");

        bool result = false;

        if (ImGui::BeginCombo(pLabel, it->second))
        {
            for (const auto& selectable : pOptions)
            {
                auto& [type, label] = selectable;

                if (ImGui::Selectable(label))
                {
                    pCurrent = type;
                    result = true;
                }
            }
            ImGui::EndCombo();
        }
        return result;
    }

} // namespace ImGui

namespace UI
{
    Editor::Editor(System::TextureSystem& pTextureSystem, System::MeshSystem& pMeshSystem, System::SceneSystem& pSceneSystem, System::CollisionSystem& pCollisionSystem, OpenGL::OpenGLRenderer& pOpenGLRenderer)
        : mTextureSystem{pTextureSystem}
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
        Platform::Core::mMouseButtonEvent.subscribe(this, &Editor::onMousePressed);

        auto writeTriangleConstructor = [](const Geometry::Triangle& pTriangle)
        {
            auto constructor = std::format("Geometry::Triangle(glm::vec3({}f, {}f, {}f), glm::vec3({}f, {}f, {}f), glm::vec3({}f, {}f, {}f))"
            , pTriangle.mPoint1.x, pTriangle.mPoint1.y, pTriangle.mPoint1.z
            , pTriangle.mPoint2.x, pTriangle.mPoint2.y, pTriangle.mPoint2.z
            , pTriangle.mPoint3.x, pTriangle.mPoint3.y, pTriangle.mPoint3.z);
            std::cout << constructor << std::endl;
        };

        initialiseStyling();
    }

    void Editor::onMousePressed(const Platform::MouseButton& pMouseButton, const Platform::Action& pAction)
    {
        if (!Platform::Core::UICapturingMouse() && !Platform::Core::getWindow().capturingMouse())
        {
            switch (pMouseButton)
            {
                case Platform::MouseButton::MOUSE_LEFT:
                {
                    if (pAction == Platform::Action::PRESS)
                    {
                        auto cursorRay          = mOpenGLRenderer.getCursorWorldRay();
                        auto entitiesUnderMouse = mCollisionSystem.getEntitiesAlongRay(cursorRay);

                        if (!entitiesUnderMouse.empty())
                        {
                            std::sort(entitiesUnderMouse.begin(), entitiesUnderMouse.end(),[](const auto& left, const auto& right) { return left.second < right.second; });
                            auto entityCollided = entitiesUnderMouse.front().first;

                            mSelectedEntities.push_back(entityCollided);
                            LOG("Entity{} has been selected", entityCollided.ID);
                            }

                            const auto mouseRayCylinder  = Geometry::Cylinder(mSceneSystem.getPrimaryCamera()->get_position(), mSceneSystem.getPrimaryCamera()->get_position() + (cursorRay.mDirection * 1000.f), 0.02f);
                            mOpenGLRenderer.mDebugOptions.mCylinders.push_back(mouseRayCylinder);
                    }
                    break;
                }
                case Platform::MouseButton::MOUSE_MIDDLE:
                {
                    mOpenGLRenderer.mDebugOptions.mCylinders.clear();
                    break;
                }
                case Platform::MouseButton::MOUSE_RIGHT:
                {
                    if (pAction == Platform::Action::PRESS)
                        Platform::Core::getWindow().setInputMode(Platform::Core::getWindow().capturingMouse() ? Platform::CursorMode::NORMAL : Platform::CursorMode::CAPTURED);
                    break;
                }
            }
        }
    }

    void Editor::draw(const DeltaTime& p_duration_since_last_draw)
    {
        m_duration_between_draws.push_back(p_duration_since_last_draw);

        Platform::Core::startImGuiFrame();

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

        Platform::Core::endImGuiFrame();
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

                    ImGui::Separator();
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
            auto& window         = Platform::Core::getWindow();
            auto [width, height] = window.size();

            //ImGui::Text(("Viewport size: " + std::to_string(width) + "x" + std::to_string(height)).c_str());
            //ImGui::Text(("Aspect ratio: " + std::to_string(window.get_aspect_ratio())).c_str());
            bool VSync = window.get_VSync();
            if (ImGui::Checkbox("VSync", &VSync)) window.set_VSync(VSync);
            //ImGui::Text("View Position", mOpenGLRenderer.mViewInformation.mViewPosition);
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
                    const std::vector<std::pair<GLType::DepthTestType, const char*>> depthTestOptions =
                    {
                        { GLType::DepthTestType::Always, "Always" },
                        { GLType::DepthTestType::Never, "Never" },
                        { GLType::DepthTestType::Less, "Less" },
                        { GLType::DepthTestType::Equal, "Equal" },
                        { GLType::DepthTestType::NotEqual, "NotEqual" },
                        { GLType::DepthTestType::Greater, "Greater" },
                        { GLType::DepthTestType::LessEqual, "LessEqual" },
                        { GLType::DepthTestType::GreaterEqual, "GreaterEqual" }
                    };
                    ImGui::ComboContainer("Forced depth test type", mOpenGLRenderer.mDebugOptions.mForcedDepthTestType, depthTestOptions );
                }
                if (!mOpenGLRenderer.mDebugOptions.mForceDepthTestType) ImGui::EndDisabled();
            }
            { // Blending options
                ImGui::Checkbox("Force blend type", &mOpenGLRenderer.mDebugOptions.mForceBlendType);

                const std::vector<std::pair<GLType::BlendFactorType, const char*>> blendOptions =
                {
                    { GLType::BlendFactorType::Zero, "Zero" },
                    { GLType::BlendFactorType::One, "One" },
                    { GLType::BlendFactorType::SourceColour, "SourceColour" },
                    { GLType::BlendFactorType::OneMinusSourceColour, "OneMinusSourceColour" },
                    { GLType::BlendFactorType::DestinationColour, "DestinationColour" },
                    { GLType::BlendFactorType::OneMinusDestinationColour, "OneMinusDestinationColour" },
                    { GLType::BlendFactorType::SourceAlpha, "SourceAlpha" },
                    { GLType::BlendFactorType::OneMinusSourceAlpha, "OneMinusSourceAlpha" },
                    { GLType::BlendFactorType::DestinationAlpha, "DestinationAlpha" },
                    { GLType::BlendFactorType::OneMinusDestinationAlpha, "OneMinusDestinationAlpha" },
                    { GLType::BlendFactorType::ConstantColour, "ConstantColour" },
                    { GLType::BlendFactorType::OneMinusConstantColour, "OneMinusConstantColour" },
                    { GLType::BlendFactorType::ConstantAlpha, "ConstantAlpha" },
                    { GLType::BlendFactorType::OneMinusConstantAlpha, "OneMinusConstantAlpha" }
                };
                if (!mOpenGLRenderer.mDebugOptions.mForceBlendType) ImGui::BeginDisabled();
                {
                    ImGui::ComboContainer("Source", mOpenGLRenderer.mDebugOptions.mForcedSourceBlendType, blendOptions );
                    ImGui::ComboContainer("Destination", mOpenGLRenderer.mDebugOptions.mForcedDestinationBlendType, blendOptions );
                }
                if (!mOpenGLRenderer.mDebugOptions.mForceBlendType) ImGui::EndDisabled();
            }
            { // Cull face options
                const std::vector<std::pair<GLType::CullFacesType, const char*>> cullFaceOptions =
                {
                    { GLType::CullFacesType::Back, "Back" },
                    { GLType::CullFacesType::Front, "Front" },
                    { GLType::CullFacesType::FrontAndBack, "FrontAndBack" }
                };

                ImGui::Checkbox("Force cull face type", &mOpenGLRenderer.mDebugOptions.mForceCullFacesType);
                if (!mOpenGLRenderer.mDebugOptions.mForceCullFacesType) ImGui::BeginDisabled();
                {
                    ImGui::ComboContainer("Forced cull faces type", mOpenGLRenderer.mDebugOptions.mForcedCullFacesType, cullFaceOptions );
                }
                if (!mOpenGLRenderer.mDebugOptions.mForceCullFacesType) ImGui::EndDisabled();

                const std::vector<std::pair<GLType::FrontFaceOrientation, const char*>> frontFaceOrientationOptions =
                {
                    { GLType::FrontFaceOrientation::Clockwise, "Clockwise" },
                    { GLType::FrontFaceOrientation::CounterClockwise, "CounterClockwise" }
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
                mOpenGLRenderer.mDebugOptions.mForcedDepthTestType = GLType::DepthTestType::Less;
                mOpenGLRenderer.mDebugOptions.mForceBlendType = false;
                mOpenGLRenderer.mDebugOptions.mForcedSourceBlendType = GLType::BlendFactorType::SourceAlpha;
                mOpenGLRenderer.mDebugOptions.mForcedDestinationBlendType = GLType::BlendFactorType::OneMinusSourceAlpha;
                mOpenGLRenderer.mDebugOptions.mForceCullFacesType = false;
                mOpenGLRenderer.mDebugOptions.mForcedCullFacesType = GLType::CullFacesType::Back;
                mOpenGLRenderer.mDebugOptions.mForceFrontFaceOrientationType = false;
                mOpenGLRenderer.mDebugOptions.mForcedFrontFaceOrientationType = GLType::FrontFaceOrientation::CounterClockwise;
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