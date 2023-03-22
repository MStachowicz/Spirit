#include "Application.hpp"

Application::Application()
    : mTextureSystem()
    , mMeshSystem(mTextureSystem)
    , mSceneSystem(mTextureSystem, mMeshSystem)
    , mOpenGLRenderer(mSceneSystem, mMeshSystem, mTextureSystem)
    , mCollisionSystem(mSceneSystem, mMeshSystem)
    , mPhysicsSystem(mSceneSystem, mCollisionSystem)
    , mInputSystem(mSceneSystem)
    , mEditor(mTextureSystem, mMeshSystem, mSceneSystem, mCollisionSystem, mOpenGLRenderer)
{
    Logger::s_editor_sink = &mEditor;
}

void Application::simulationLoop()
{
    while (Platform::Core::hasWindow())
    {
        switch (mPhysicsTicksPerSecond)
        {
            case 30:  simulationLoop<30>();  break;
            case 60:  simulationLoop<60>();  break;
            case 90:  simulationLoop<90>();  break;
            case 120: simulationLoop<120>(); break;
            default: throw std::logic_error("Invalid value assigned to mPhysicsTicksPerSecond"); break;
        }

        // After exiting a simulation loop we may have requested a physics timestep change.
        // Reset this flag to not exit the next simulationLoop when looping back around this While().
        mPhysicsTimeStepChanged = false;
    }
}