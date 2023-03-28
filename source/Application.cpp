#include "Application.hpp"

Application::Application()
    : mTextureSystem{}
    , mMeshSystem{mTextureSystem}
    , mSceneSystem{mTextureSystem, mMeshSystem}
    , mOpenGLRenderer{mSceneSystem, mMeshSystem, mTextureSystem}
    , mCollisionSystem{mSceneSystem, mMeshSystem}
    , mPhysicsSystem{mSceneSystem, mCollisionSystem}
    , mInputSystem{mSceneSystem}
    , mEditor{mTextureSystem, mMeshSystem, mSceneSystem, mCollisionSystem, mOpenGLRenderer}
    , mSimulationLoopParamsChanged{false}
    , mPhysicsTicksPerSecond{60}
    , mRenderTicksPerSecond{120}
    , maxFrameDelta{std::chrono::milliseconds(250)}
{
    Logger::s_editor_sink = &mEditor;
}

void Application::simulationLoop()
{
    while (Platform::Core::hasWindow())
    {
        switch (mPhysicsTicksPerSecond)
        {
            case 30:
            {
                switch (mRenderTicksPerSecond)
                {
                    case 30:
                    {
                        simulationLoop<30, 30>();
                        break;
                    }
                    case 60:
                    {
                        simulationLoop<30, 60>();
                        break;
                    }
                    case 90:
                    {
                        simulationLoop<30, 90>();
                        break;
                    }
                    case 120:
                    {
                        simulationLoop<30, 120>();
                        break;
                    }
                }
                break;
            }
            case 60:
            {
                switch (mRenderTicksPerSecond)
                {
                    case 30:
                    {
                        simulationLoop<60, 30>();
                        break;
                    }
                    case 60:
                    {
                        simulationLoop<60, 60>();
                        break;
                    }
                    case 90:
                    {
                        simulationLoop<60, 90>();
                        break;
                    }
                    case 120:
                    {
                        simulationLoop<60, 120>();
                        break;
                    }
                }
                break;
            }
            case 90:
            {
                switch (mRenderTicksPerSecond)
                {
                    case 30:
                    {
                        simulationLoop<90, 30>();
                        break;
                    }
                    case 60:
                    {
                        simulationLoop<90, 60>();
                        break;
                    }
                    case 90:
                    {
                        simulationLoop<90, 90>();
                        break;
                    }
                    case 120:
                    {
                        simulationLoop<90, 120>();
                        break;
                    }
                }
                break;
            }
            case 120:
            {
                switch (mRenderTicksPerSecond)
                {
                    case 30:
                    {
                        simulationLoop<120, 30>();
                        break;
                    }
                    case 60:
                    {
                        simulationLoop<120, 60>();
                        break;
                    }
                    case 90:
                    {
                        simulationLoop<120, 90>();
                        break;
                    }
                    case 120:
                    {
                        simulationLoop<120, 120>();
                        break;
                    }
                }
                break;
            }
            default: ASSERT(false, "Invalid value assigned to mPhysicsTicksPerSecond"); break;
        }

        // After exiting a simulation loop we may have requested a physics timestep change.
        // Reset this flag to not exit the next simulationLoop when looping back around this While().
        mSimulationLoopParamsChanged = false;
    }
}