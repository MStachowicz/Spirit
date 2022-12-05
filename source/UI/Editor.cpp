#include "Editor.hpp"

// ECS
#include "Storage.hpp"

// SYSTEM
#include "SceneSystem.hpp"

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

// UTILITY
#include "Logger.hpp"

// IMGUI
#define IMGUI_USER_CONFIG "ImGuiConfig.hpp"
#include "imgui.h"

namespace UI
{
    Editor::Editor(System::SceneSystem& pSceneSystem, OpenGL::OpenGLRenderer& pOpenGLRenderer)
        : mDrawCount{0}
        , mSceneSystem{pSceneSystem}
        , mOpenGLRenderer{pOpenGLRenderer}
    {}

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