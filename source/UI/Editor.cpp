#include "Editor.hpp"

// ECS
#include "Storage.hpp"

// SYSTEM
#include "SceneSystem.hpp"
#include "CollisionSystem.hpp"

// COMPONENT
#include "Camera.hpp"
#include "Collider.hpp"
#include "DirectionalLight.hpp"
#include "Mesh.hpp"
#include "PointLight.hpp"
#include "RigidBody.hpp"
#include "SpotLight.hpp"
#include "Transform.hpp"

// OPENGL
#include "OpenGLRenderer.hpp"

// PLATFORM
#include "Core.hpp"
#include "InputDefinitions.hpp"

// UTILITY
#include "Logger.hpp"

// IMGUI
#define IMGUI_USER_CONFIG "ImGuiConfig.hpp"
#include "imgui.h"

namespace UI
{
    Editor::Editor(System::SceneSystem& pSceneSystem, System::CollisionSystem& pCollisionSystem, OpenGL::OpenGLRenderer& pOpenGLRenderer)
        : mDrawCount{0}
        , mSceneSystem{pSceneSystem}
        , mCollisionSystem{pCollisionSystem}
        , mOpenGLRenderer{pOpenGLRenderer}
    {
        Platform::Core::mMouseButtonEvent.subscribe(std::bind(&Editor::onMousePressed, this, std::placeholders::_1, std::placeholders::_2));
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
            auto& scene = mSceneSystem.getCurrentScene();
            scene.foreachEntity([&](ECS::EntityID& pEntity)
            {
                const std::string title = "Entity " + std::to_string(pEntity);
                if (ImGui::TreeNode(title.c_str()))
                {
                    if (scene.hasComponents<Component::Transform>(pEntity))
                        scene.getComponentMutable<Component::Transform&>(pEntity).DrawImGui();
                    if (scene.hasComponents<Component::Collider>(pEntity))
                        scene.getComponentMutable<Component::Collider&>(pEntity).DrawImGui();
                    if (scene.hasComponents<Component::RigidBody>(pEntity))
                        scene.getComponentMutable<Component::RigidBody&>(pEntity).DrawImGui();
                    if (scene.hasComponents<Component::MeshDraw>(pEntity))
                        scene.getComponentMutable<Component::MeshDraw&>(pEntity).DrawImGui();
                    if (scene.hasComponents<Component::Camera>(pEntity))
                        scene.getComponentMutable<Component::Camera&>(pEntity).DrawImGui();
                    if (scene.hasComponents<Component::DirectionalLight>(pEntity))
                        scene.getComponentMutable<Component::DirectionalLight&>(pEntity).DrawImGui();
                    if (scene.hasComponents<Component::SpotLight>(pEntity))
                        scene.getComponentMutable<Component::SpotLight&>(pEntity).DrawImGui();
                    if (scene.hasComponents<Component::PointLight>(pEntity))
                        scene.getComponentMutable<Component::PointLight&>(pEntity).DrawImGui();

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