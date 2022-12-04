#pragma once

#include <vector>
#include <chrono>

namespace System
{
    class SceneSystem;
}

class Renderer
{
public:
    Renderer(System::SceneSystem& pSceneSystem);

    void draw();

    void renderImGui();
    void drawEntityPanel();

    int mDrawCount;
    int mTargetFPS; // Independently of physics, the number of frames the renderer will aim to draw per second.

private:
    System::SceneSystem& mSceneSystem;

    bool mShowFPSPlot;
    int mFPSSampleSize;           // The number of frames used to graph the FPS and calculate the average.
    float mAverageFPS;            // The average fps over the last mFPSSampleSize frames.
    std::vector<float> mFPSTimes; // Holds the last mFPSSampleSize frame times.
};