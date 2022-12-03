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
#include "Geometry.hpp"
#include "Logger.hpp"
#include "Stopwatch.hpp"

// IMGUI
#define IMGUI_USER_CONFIG "ImGuiConfig.hpp"
#include "imgui.h"

// STD
#include <chrono>
#include <cmath>
#include <numeric>

Renderer::Renderer(System::SceneSystem& pSceneSystem)
    : mDrawCount(0)
    , mTargetFPS(60)
    , mSceneSystem{pSceneSystem}
    , mRenderImGui(true)
    , mShowFPSPlot(false)
    , mUseRawPerformanceData(false)
    , mDataSmoothingFactor(0.1f)
    , mFPSSampleSize(120)
    , mAverageFPS(0)
    , mCurrentFPS(0)
    , mTimeSinceLastDraw(0)
    , mImGuiRenderTimeTakenMS(0)
    , mDrawTimeTakenMS(0)
{}

void Renderer::onFrameStart(const std::chrono::microseconds& pTimeSinceLastDraw)
{
    mTimeSinceLastDraw = static_cast<float>(pTimeSinceLastDraw.count()) / 1000.0f; // Convert microseconds to milliseconds

    if (mUseRawPerformanceData)
        mCurrentFPS = 1.0f / (static_cast<float>((pTimeSinceLastDraw.count()) / 1000000.0f));
    else
        mCurrentFPS = (mDataSmoothingFactor * (1.0f / (static_cast<float>((pTimeSinceLastDraw.count()) / 1000000.0f)))) + (1.0f - mDataSmoothingFactor) * mCurrentFPS;
}

void Renderer::draw(const std::chrono::microseconds& pTimeSinceLastDraw)
{
    Utility::Stopwatch stopwatch;

    onFrameStart(pTimeSinceLastDraw);
    renderImGui();

    mDrawCount++;
    mDrawTimeTakenMS = stopwatch.getTime<std::milli, float>();
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


void Renderer::renderImGui()
{
    // Render all ImGui from here.
    Utility::Stopwatch stopWatch;

    if (mRenderImGui)
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
    }

    // Regardless of mRenderImGui, we call newImGuiFrame() and renderImGuiFrame() to allow showing performance window.
    if (ImGui::Begin("Performance"))
    {
        // This is showing the last frame's RenderTimeTaken since the update has to happen after renderImGuiFrame below.
        // TODO: Add above comment as a help marker.
        ImGui::Text("ImGui render took: %.3fms", mImGuiRenderTimeTakenMS);
        ImGui::Text("Render took: %.3fms", mDrawTimeTakenMS);
        ImGui::Text("Frame time: %.3f ms", mTimeSinceLastDraw);

        ImGui::Separator();
        ImGui::Text("Target FPS:%d", mTargetFPS);
        ImGui::Text("FPS:");

        ImVec4 colour;
        if (mCurrentFPS >= mTargetFPS * 0.99f)
            colour = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        else if (mCurrentFPS <= mTargetFPS * 0.5f)
            colour = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        else
            colour = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        ImGui::SameLine();
        ImGui::TextColored(colour, "%.0f\t", mCurrentFPS);
        ImGui::SameLine();
        ImGui::Checkbox("Show plot", &mShowFPSPlot);
        if (mShowFPSPlot)
        {
            plotFPSTimes();

            // When changing mFPSSampleSize we have to clear the excess FPS entries in the start of the vector.
            if (ImGui::SliderInt("FPS frame sample size", &mFPSSampleSize, 1, 1000))
                if (mFPSSampleSize < mFPSTimes.size())
                    mFPSTimes.erase(mFPSTimes.begin(), mFPSTimes.end() - mFPSSampleSize); // O(n) mFPSTimes.erase linear with mFPSSampleSize
        }

        if (ImGui::TreeNode("Options"))
        {
            ImGui::Checkbox("Render ImGui", &mRenderImGui);
            ImGui::Checkbox("Use raw data", &mUseRawPerformanceData); // Whether we use smoothing for the incoming values of mCurrentFPS.
            if (!mUseRawPerformanceData)
            {
                ImGui::SameLine();
                ImGui::SliderFloat("FPS smoothing factor", &mDataSmoothingFactor, 0.f, 1.f);
            }

            ImGui::TreePop();
        }
    }
    ImGui::End();

    mImGuiRenderTimeTakenMS = stopWatch.getTime<std::milli, float>();
}

void Renderer::plotFPSTimes()
{
    // We keep a list of mFPSTimes sampling the mCurrentFPS at every Renderer::Draw.
    // mAverageFPS gives the average FPS in the mFPSSampleSize of mFPSTimes.
    // When mFPSTimes is full, clears the first entry to allow ring buffer style push_back for output using ImGui::PlotLines
    if (mFPSTimes.size() <= mFPSSampleSize)
        mFPSTimes.push_back(mCurrentFPS);
    else
    {
        mFPSTimes.erase(mFPSTimes.begin()); // O(n) mFPSTimes.erase linear with mFPSTimes.size()
        mFPSTimes.push_back(mCurrentFPS);
    }
    mAverageFPS = std::reduce(mFPSTimes.begin(), mFPSTimes.end()) / static_cast<float>(mFPSTimes.size()); // O(?) reduce faster than accumulate, per frame
    ImGui::PlotLines("", &mFPSTimes[0], static_cast<int>(mFPSTimes.size()), 0, ("Avg:" + std::to_string(std::round(mAverageFPS))).c_str(), 0.0f, mTargetFPS * 1.25f, ImVec2(ImGui::GetWindowWidth(), mTargetFPS * 1.25f));
}