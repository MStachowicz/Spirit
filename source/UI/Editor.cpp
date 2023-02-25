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

// IMGUI
#define IMGUI_USER_CONFIG "ImGuiConfig.hpp"
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

    void Text(const char* pLabel, const glm::vec3& pVec3)
    {
        ImGui::Text(std::format("{}: {}, {}, {}", pLabel, pVec3.x, pVec3.y, pVec3.z).c_str());
    }
    void Text(const char* pLabel, const glm::mat4& pMat4)
    {
        ImGui::Text(std::format("{}:\n[{}, {}, {}, {}]\n[{}, {}, {}, {}]\n[{}, {}, {}, {}]\n[{}, {}, {}, {}]"
        , pLabel
        , pMat4[0][0], pMat4[0][1], pMat4[0][2], pMat4[0][3]
        , pMat4[1][0], pMat4[1][1], pMat4[1][2], pMat4[1][3]
        , pMat4[2][0], pMat4[2][1], pMat4[2][2], pMat4[2][3]
        , pMat4[3][0], pMat4[3][1], pMat4[3][2], pMat4[3][3]

        ).c_str());
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
        , mWindowsToDisplay{}
        , mDrawCount{0}
    {
        Platform::Core::mMouseButtonEvent.subscribe(this, &Editor::onMousePressed);
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
                            LOG_INFO("Entity{} has been selected", entityCollided);
                            }

                            const auto mouseRayCylinder  = Geometry::Cylinder(mSceneSystem.getPrimaryCamera()->getPosition(), mSceneSystem.getPrimaryCamera()->getPosition() + (cursorRay.mDirection * 1000.f), 0.02f);
                            mOpenGLRenderer.debugCylinders.push_back(mouseRayCylinder);
                    }
                    break;
                }
                case Platform::MouseButton::MOUSE_MIDDLE:
                {
                    mOpenGLRenderer.debugCylinders.clear();
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

    void Editor::draw()
    {
        Platform::Core::startImGuiFrame();

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("View"))
            {
                ImGui::MenuItem("Entity hierarchy", NULL, &mWindowsToDisplay.Entity);

                if (ImGui::BeginMenu("Debug"))
                {
                    ImGui::MenuItem("Performance", NULL, &mWindowsToDisplay.Performance);
                    ImGui::MenuItem("Graphics", NULL, &mWindowsToDisplay.Graphics);

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
            ImGui::EndMenuBar();
        }

        if (mWindowsToDisplay.Entity)           drawEntityPanel();
        if (mWindowsToDisplay.Performance)      drawPerformancePanel();
        if (mWindowsToDisplay.Graphics)         drawGraphicsPanel();
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

    void Editor::drawEntityPanel()
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
                    {
                        auto& camera = scene.getComponentMutable<Component::Camera>(pEntity);
                        if (ImGui::TreeNode("Camera"))
                        {
                            ImGui::Text("Position", camera.getPosition());
                            ImGui::Text("Look direction", camera.getForwardDirection());
                            ImGui::Text("View matrix", camera.getViewMatrix());
                            ImGui::TreePop();
                        }
                    }
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
                            const std::string currentDiffuse  = textureComponent.mDiffuse.has_value() ? textureComponent.mDiffuse.value()->mName : "None";
                            const std::string currentSpecular = textureComponent.mSpecular.has_value() ? textureComponent.mSpecular.value()->mName : "None";

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

    void Editor::drawGraphicsPanel()
    {
        if (ImGui::Begin("Graphics", &mWindowsToDisplay.Graphics))
            {
                mOpenGLRenderer.renderImGui();
        }
        ImGui::End();
    }

    void Editor::drawPerformancePanel()
    {
        if (ImGui::Begin("Performance", &mWindowsToDisplay.Performance))
        {
        }
        ImGui::End();
    }
} // namespace UI