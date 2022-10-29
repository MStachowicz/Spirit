#pragma once

#include "Camera.hpp"

#include "OpenGLRenderer.hpp"

// STD
#include <vector>

namespace ECS
{
    class EntityManager;
    class Entity;
} // namespace ECS
namespace Manager
{
    class MeshManager;
    class TextureManager;
} // namespace Manager

class Renderer
{
public:
    Renderer(ECS::EntityManager& pEntityManager, const Manager::TextureManager& pTextureManager, const Manager::MeshManager& pMeshManager);

    void onFrameStart(const std::chrono::microseconds& pTimeSinceLastDraw);
    void draw(const std::chrono::microseconds& pTimeSinceLastDraw);

    void renderImGui();
    void plotFPSTimes();

    int mDrawCount;
    int mTargetFPS; // Independently of physics, the number of frames the renderer will aim to draw per second.
    Camera& getCamera() { return mCamera; }

private:
    ECS::EntityManager& mEntityManager;
    const Manager::TextureManager& mTextureManager;
    const Manager::MeshManager& mMeshManager;

    OpenGL::OpenGLRenderer mOpenGLRenderer;
    Camera mCamera;

    bool mRenderImGui; // Toggle displaying all Zephyr ImGui except the Performance window.
    bool mRenderLightPositions;
    bool mShowFPSPlot; // Whether we are displaying the

    bool mUseRawPerformanceData; // Whether the values displayed in Performance window will be averaged for smoother display.
    float mDataSmoothingFactor;  // The factor to smooth the performance data with. 0 = quickly discard old value, 1= keep effect of old values longer

    int mFPSSampleSize;           // The number of frames used to graph the FPS and calculate the average.
    std::vector<float> mFPSTimes; // Holds the last mFPSSampleSize frame times.
    float mAverageFPS;            // The average fps over the last mFPSSampleSize frames.
    float mCurrentFPS;            // The FPS in the current frame.

    float mTimeSinceLastDraw;
    float mImGuiRenderTimeTakenMS; // Time taken to render all ImGui. 1 Frame late to allow output to Performance window.
    float mDrawTimeTakenMS;        // Time taken to complete Renderer::Draw().
};