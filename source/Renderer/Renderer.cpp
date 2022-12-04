#include "Renderer.hpp"

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

// UTILITY
#include "Logger.hpp"

// IMGUI
#define IMGUI_USER_CONFIG "ImGuiConfig.hpp"
#include "imgui.h"

Renderer::Renderer(System::SceneSystem& pSceneSystem)
    : mDrawCount(0)
    , mTargetFPS(60)
    , mSceneSystem{pSceneSystem}
    , mShowFPSPlot(false)
    , mFPSSampleSize(120)
    , mAverageFPS(0)
{}

void Renderer::draw()
{
    if (ImGui::Begin("ImGui options"))
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
    }
    ImGui::End();

    drawEntityPanel();

    ImGui::ShowDemoWindow();
    ImGui::ShowMetricsWindow();

    // Regardless of mRenderImGui, we call newImGuiFrame() and renderImGuiFrame() to allow showing performance window.
    if (ImGui::Begin("Performance"))
    {
        ImGui::Text("Target FPS:%d", mTargetFPS);

        ImGui::Text("FPS:");
        ImVec4 colour;
        if (mAverageFPS >= mTargetFPS * 0.99f)
            colour = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        else if (mAverageFPS <= mTargetFPS * 0.5f)
            colour = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        else
            colour = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        ImGui::SameLine();
        ImGui::TextColored(colour, "%.0f\t", mAverageFPS);


        ImGui::Checkbox("Show plot", &mShowFPSPlot);
        if (mShowFPSPlot)
        {
            // When changing mFPSSampleSize we have to clear the excess FPS entries in the start of the vector.
            if (ImGui::SliderInt("FPS frame sample size", &mFPSSampleSize, 1, 1000))
                if (mFPSSampleSize < mFPSTimes.size())
                    mFPSTimes.erase(mFPSTimes.begin(), mFPSTimes.end() - mFPSSampleSize); // O(n) mFPSTimes.erase linear with mFPSSampleSize
        }
    }
    ImGui::End();

    mDrawCount++;
}

void Renderer::drawEntityPanel()
{
    if (ImGui::Begin("Entity options"))
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