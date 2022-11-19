#pragma once

#include "OpenGLRenderer.hpp"

#include <vector>

namespace ECS
{
    class Storage;
}
namespace System
{
    class MeshSystem;
    class TextureSystem;
}

class Renderer
{
public:
    Renderer(ECS::Storage& pStorage, const System::TextureSystem& pTextureSystem, const System::MeshSystem& pMeshSystem);

    void onFrameStart(const std::chrono::microseconds& pTimeSinceLastDraw);
    void draw(const std::chrono::microseconds& pTimeSinceLastDraw);

    void renderImGui();
    void drawEntityPanel();
    void plotFPSTimes();

    int mDrawCount;
    int mTargetFPS; // Independently of physics, the number of frames the renderer will aim to draw per second.

private:
    ECS::Storage& mStorage;
    const System::TextureSystem& mTextureSystem;
    const System::MeshSystem& mMeshSystem;
    OpenGL::OpenGLRenderer mOpenGLRenderer;

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