#include "Renderer.hpp"

// ECS
#include "EntityManager.hpp"
#include "EventDispatcher.hpp"

// IMGUI
#define IMGUI_USER_CONFIG "ImGuiConfig.hpp"
#include "imgui.h"

// LOGGER
#include "Logger.hpp"
// UTILITY
#include "Timer.hpp"

// STD
#include <chrono>
#include <cmath>
#include <numeric>

Renderer::Renderer(ECS::EntityManager& pEntityManager)
    : mDrawCount(0)
    , mTargetFPS(60)
    , mTextureManager()
    , mMeshManager(mTextureManager)
    , mEntityManager(pEntityManager)
    , mOpenGLRenderer(mEntityManager)
    , mCamera(glm::vec3(0.0f, 1.7f, 7.0f), std::bind(&OpenGL::OpenGLRenderer::setView, &mOpenGLRenderer, std::placeholders::_1), std::bind(&OpenGL::OpenGLRenderer::setViewPosition, &mOpenGLRenderer, std::placeholders::_1))
    , mRenderImGui(true)
    , mRenderLightPositions(true)
    , mShowFPSPlot(false)
    , mUseRawPerformanceData(false)
    , mDataSmoothingFactor(0.1f)
    , mFPSSampleSize(120)
    , mAverageFPS(0)
    , mCurrentFPS(0)
    , mTimeSinceLastDraw(0)
    , mImGuiRenderTimeTakenMS(0)
    , mDrawTimeTakenMS(0)
{
    mMeshManager.ForEach([this](const auto& mesh)
                         { mOpenGLRenderer.initialiseMesh(mesh); }); // Depends on mShaders being initialised.
    mTextureManager.ForEach([this](const auto& texture)
                            { mOpenGLRenderer.initialiseTexture(texture); });
    mTextureManager.ForEachCubeMap([this](const auto& cubeMap)
                                   { mOpenGLRenderer.initialiseCubeMap(cubeMap); });

    // Subscribe the listeners here as opposed to inside the OpenGLRenderer constructor to maintain the const& qualifier.
    // OpenGLRenderer is a listener and shouldnt have the option to edit the data layer.
    mEntityManager.mEntityCreatedEvent.Subscribe(std::bind(&OpenGL::OpenGLRenderer::onEntityCreated, &mOpenGLRenderer, std::placeholders::_1, std::placeholders::_2));
    mEntityManager.mEntityRemovedEvent.Subscribe(std::bind(&OpenGL::OpenGLRenderer::onEntityRemoved, &mOpenGLRenderer, std::placeholders::_1, std::placeholders::_2));

    mEntityManager.mTransforms.mComponentAddedEvent.Subscribe(std::bind(&OpenGL::OpenGLRenderer::onTransformComponentAdded, &mOpenGLRenderer, std::placeholders::_1, std::placeholders::_2));
    mEntityManager.mTransforms.mComponentChangedEvent.Subscribe(std::bind(&OpenGL::OpenGLRenderer::onTransformComponentChanged, &mOpenGLRenderer, std::placeholders::_1, std::placeholders::_2));
    mEntityManager.mTransforms.mComponentRemovedEvent.Subscribe(std::bind(&OpenGL::OpenGLRenderer::onTransformComponentRemoved, &mOpenGLRenderer, std::placeholders::_1));

    mEntityManager.mMeshes.mComponentAddedEvent.Subscribe(std::bind(&OpenGL::OpenGLRenderer::onMeshComponentAdded, &mOpenGLRenderer, std::placeholders::_1, std::placeholders::_2));
    // mEntityManager.mMeshes.mComponentChangedEvent.Subscribe(std::bind(&OpenGL::OpenGLRenderer::onMeshComponentChanged, &mOpenGLRenderer, std::placeholders::_1, std::placeholders::_2));
    mEntityManager.mMeshes.mComponentRemovedEvent.Subscribe(std::bind(&OpenGL::OpenGLRenderer::onMeshComponentRemoved, &mOpenGLRenderer, std::placeholders::_1));

    static const float floorSize           = 50.f;
    static const size_t grassCount         = 500;
    static const bool randomGrassPlacement = false;
    { // Floor
        auto& entity = mEntityManager.CreateEntity();

        Data::Transform transform;
        transform.mPosition;
        transform.mRotation.x = -90.f;
        transform.mScale      = glm::vec3(floorSize);
        mEntityManager.mTransforms.Add(entity, transform);

        Data::MeshDraw mesh;
        mesh.mID                  = mMeshManager.getMeshID("Quad");
        mesh.mDrawStyle           = Data::DrawStyle::LightMap;
        mesh.mDiffuseTextureID    = mTextureManager.getTextureID("grassTile");
        mesh.mSpecularTextureID   = mTextureManager.getTextureID("black");
        mesh.mShininess           = 128.f;
        mesh.mTextureRepeatFactor = 20.f;
        mEntityManager.mMeshes.Add(entity, mesh);
    }
    { // Cubes
        std::array<glm::vec3, 10> cubePositions = {
            glm::vec3(3.0f, 0.5f, -3.0f),
            glm::vec3(-1.3f, 0.5f, -1.5f),
            glm::vec3(1.5f, 0.5f, -2.5f),
            glm::vec3(-1.5f, 0.5f, -2.5f),
            glm::vec3(2.4f, 0.5f, -3.5f),
            glm::vec3(1.5f, 0.5f, -5.5f),
            glm::vec3(-1.7f, 0.5f, -7.5f),
            glm::vec3(1.3f, 0.5f, -8.5f),
            glm::vec3(-3.8f, 0.5f, -12.3f),
            glm::vec3(2.0f, 0.5f, -15.0f)};
        for (size_t i = 0; i < cubePositions.size(); i++)
        {
            auto& entity = mEntityManager.CreateEntity();

            Data::Transform transform;
            transform.mPosition = cubePositions[i];
            mEntityManager.mTransforms.Add(entity, transform);

            Data::MeshDraw mesh;
            mesh.mID                = mMeshManager.getMeshID("cube");
            mesh.mName              = "3DCube";
            mesh.mDrawStyle         = Data::DrawStyle::LightMap;
            mesh.mDiffuseTextureID  = mTextureManager.getTextureID("metalContainerDiffuse");
            mesh.mSpecularTextureID = mTextureManager.getTextureID("metalContainerSpecular");
            mesh.mShininess         = 64.f;
            mEntityManager.mMeshes.Add(entity, mesh);
        }
    }
    { // Backpack
        auto& entity = mEntityManager.CreateEntity();

        Data::Transform transform;
        transform.mPosition = glm::vec3(-3.0f, 1.0f, 1.f);
        transform.mScale    = glm::vec3(0.5f);
        mEntityManager.mTransforms.Add(entity, transform);

        Data::MeshDraw mesh;
        mesh.mID                = mMeshManager.getMeshID("backpack");
        mesh.mDrawStyle         = Data::DrawStyle::LightMap;
        mesh.mDiffuseTextureID  = mTextureManager.getTextureID("diffuse");
        mesh.mSpecularTextureID = mTextureManager.getTextureID("specular");
        mesh.mShininess         = 64.f;
        mEntityManager.mMeshes.Add(entity, mesh);

    }
    { // Xian
        auto& entity = mEntityManager.CreateEntity();

        Data::Transform transform;
        transform.mPosition = glm::vec3(8.0f, 10.0f, 0.0f);
        transform.mRotation = glm::vec3(-10.0f, 230.0f, -15.0f);
        transform.mScale    = glm::vec3(0.4f);
        mEntityManager.mTransforms.Add(entity, transform);

        Data::MeshDraw mesh;
        mesh.mID                = mMeshManager.getMeshID("xian");
        mesh.mDrawStyle         = Data::DrawStyle::LightMap;
        mesh.mDiffuseTextureID  = mTextureManager.getTextureID("Base_Color");
        mesh.mSpecularTextureID = mTextureManager.getTextureID("black");
        mesh.mShininess         = 64.f;
        mEntityManager.mMeshes.Add(entity, mesh);
    }
    { // Billboard grass
        std::array<glm::vec3, grassCount> grassPositions;
        {
            if (randomGrassPlacement)
            {
                std::array<float, grassCount> randomX;
                util::fillRandomNumbers(-floorSize, floorSize, randomX);
                std::array<float, grassCount> randomZ;
                util::fillRandomNumbers(-floorSize, floorSize, randomZ);
                for (size_t i = 0; i < grassCount; i++)
                    grassPositions[i] = glm::vec3(randomX[i], 0.f, randomZ[i]);
            }
            else
            {
                const float distance = 1.f;
                float x              = -1;
                float z              = 0;
                for (size_t i = 0; i < grassCount; i++)
                {
                    x += distance;
                    if (x > floorSize)
                    {
                        x = -floorSize;

                        z += distance;
                        if (z > floorSize)
                            z = -floorSize;
                    }

                    grassPositions[i] = glm::vec3(x, 0.f, z);
                }
            }
        }

        std::array<float, grassCount> randomY;
        util::fillRandomNumbers(0.2f, 0.6f, randomY);
        for (size_t i = 0; i < grassCount; i++)
        {
            auto& entity = mEntityManager.CreateEntity();

            Data::Transform transform;
            transform.mScale    = glm::vec3(0.2f, randomY[i], 0.2f);
            transform.mPosition = grassPositions[i];
            transform.mPosition.y += transform.mScale.y;
            mEntityManager.mTransforms.Add(entity, transform);

            Data::MeshDraw mesh;
            mesh.mID        = mMeshManager.getMeshID("Quad");
            mesh.mDrawStyle = Data::DrawStyle::Textured;
            mesh.mTexture1  = mTextureManager.getTextureID("grassBillboard");
            mEntityManager.mMeshes.Add(entity, mesh);
        }
    }
    { // Windows
        std::array<glm::vec3, 5> windowPositions = {
            glm::vec3(-1.5f, 0.0f, 1.48f),
            glm::vec3(1.5f, 0.0f, 1.51f),
            glm::vec3(0.0f, 0.0f, 1.7f),
            glm::vec3(-0.3f, 0.0f, 1.3f),
            glm::vec3(0.5f, 0.0f, 1.6f)};

        for (const auto& position : windowPositions)
        {
            auto& entity = mEntityManager.CreateEntity();

            Data::Transform transform;
            transform.mScale    = glm::vec3(0.2f);
            transform.mPosition = position;
            transform.mPosition.y += transform.mScale.y;
            mEntityManager.mTransforms.Add(entity, transform);

            Data::MeshDraw mesh;
            mesh.mID        = mMeshManager.getMeshID("Quad");
            mesh.mDrawStyle = Data::DrawStyle::Textured;
            mesh.mTexture1  = mTextureManager.getTextureID("window");
            mEntityManager.mMeshes.Add(entity, mesh);
        }
    }
    {     // Lights
        { // Point light
            const std::array<glm::vec3, 4> pointLightPositions = {
                glm::vec3(0.7f, 1.7f, 2.0f),
                glm::vec3(0.0f, 1.0f, -3.0f),
                glm::vec3(2.3f, 3.3f, -4.0f),
                glm::vec3(-4.0f, 2.0f, -12.0f)};
            const std::array<glm::vec3, 4> pointLightColours = {
                glm::vec3(0.f, 0.f, 1.f),
                glm::vec3(1.f),
                glm::vec3(1.f),
                glm::vec3(1.f)};

            for (size_t i = 0; i < pointLightPositions.size(); i++)
            {
                Data::PointLight pointLight;
                pointLight.mPosition = pointLightPositions[i];
                pointLight.mColour   = pointLightColours[i];
                mEntityManager.mPointLights.Add(mEntityManager.CreateEntity(), pointLight);
            }
        }

        { // Directional light
            Data::DirectionalLight directionalLight;
            directionalLight.mDirection = glm::vec3(-0.2f, -1.0f, -0.3f);
            mEntityManager.mDirectionalLights.Add(mEntityManager.CreateEntity(), directionalLight);
        }
        // Spotlight
        mEntityManager.mSpotLights.Add(mEntityManager.CreateEntity(), {});
    }
}

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
    Stopwatch stopwatch;

    onFrameStart(pTimeSinceLastDraw);
    mOpenGLRenderer.preDraw();
    mOpenGLRenderer.setupLights(mRenderLightPositions);
    mOpenGLRenderer.draw();
    mOpenGLRenderer.postDraw();

    renderImGui(); // Render ImGui last so UI is drawn over the scene.
    mOpenGLRenderer.endFrame();

    mDrawCount++;
    mDrawTimeTakenMS = stopwatch.getTime<std::milli, float>();
}

void Renderer::renderImGui()
{
    // Render all ImGui from here.
    Stopwatch stopWatch;

    mOpenGLRenderer.newImGuiFrame();
    if (mRenderImGui)
    {
        if (ImGui::Begin("Render options", nullptr))
        {
            ImGui::Checkbox("Render light positions", &mRenderLightPositions);
        }
        ImGui::End();

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

        mEntityManager.DrawImGui();
        ImGui::ShowDemoWindow();
        ImGui::ShowMetricsWindow();

        mOpenGLRenderer.renderImGui();
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

    mOpenGLRenderer.renderImGuiFrame();
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