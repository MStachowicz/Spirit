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
        : mDrawCount{0}
        , mTextureSystem{pTextureSystem}
        , mMeshSystem{pMeshSystem}
        , mSceneSystem{pSceneSystem}
        , mCollisionSystem{pCollisionSystem}
        , mOpenGLRenderer{pOpenGLRenderer}
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
                        auto entitiesUnderMouse      = mCollisionSystem.getEntitiesAlongRay(mOpenGLRenderer.getCursorWorldRay());

                        if (!entitiesUnderMouse.empty())
                        {
                            std::sort(entitiesUnderMouse.begin(), entitiesUnderMouse.end(),[](const auto& left, const auto& right) { return left.second < right.second; });
                            mSelectedEntities.push_back(entitiesUnderMouse.front().first);
                            LOG_INFO("Entity{} has been selected", entitiesUnderMouse.front().first);
                            }

                        const auto mouseRayDirection = mOpenGLRenderer.getCursorWorldDirection();
                        const auto mouseRayCylinder  = Geometry::Cylinder(mSceneSystem.getPrimaryCamera()->getPosition(), mSceneSystem.getPrimaryCamera()->getPosition() + (mouseRayDirection * 1000.f), 0.02f);
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

        drawEntityPanel();
        drawGraphicsPanel();
        drawPerformancePanel();

        Platform::Core::endImGuiFrame();
        mDrawCount++;
    }

    void Editor::drawEntityPanel()
    {
        if (ImGui::Begin("Entities"))
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
            scene.foreachEntity([&](ECS::EntityID& pEntity)
            {
                std::string title = "Entity " + std::to_string(pEntity);
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
        if (ImGui::Begin("Graphics"))
        {
            if (ImGui::TreeNode("ImGui"))
            {
                ImGuiIO& io = ImGui::GetIO();
                ImGui::Text("DisplaySize: %.fx%.f", io.DisplaySize.x, io.DisplaySize.y);
                ImGui::Text("MainViewport()->DpiScale: %.3f", ImGui::GetMainViewport()->DpiScale);
                ImGui::DragFloat("FontGlobalScale", &io.FontGlobalScale, 0.005f, 0.3f, 4.0f, "%.1f");
                ImGui::Checkbox("WantCaptureMouse", &io.WantCaptureMouse);

                if (ImGui::TreeNode("Style editor"))
                {
                    ImGui::ShowStyleEditor();
                    ImGui::TreePop();
                }

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("OpenGL"))
            {
                mOpenGLRenderer.renderImGui();
                ImGui::TreePop();
            }
        }
        ImGui::End();

        ImGui::ShowDemoWindow();
    }

    void Editor::drawPerformancePanel()
    {
        ImGui::ShowMetricsWindow();

        if (ImGui::Begin("Performance"))
        {
        }
        ImGui::End();
    }
} // namespace UI