#include "Application.hpp"


Application::Application()
    : mTextureSystem()
    , mMeshSystem(mTextureSystem)
    , mStorage()
    , mRenderer(mStorage, mTextureSystem, mMeshSystem)
    , mInput(mStorage)
    , mCollisionSystem(mStorage)
    , mPhysicsSystem(mStorage)
{}

void Application::simulationLoop()
{
    while (!mInput.closeRequested())
    {
        switch (mPhysicsTicksPerSecond)
        {
        case 30: simulationLoop<30>();
            break;
        case 60: simulationLoop<60>();
            break;
        case 90: simulationLoop<90>();
            break;
        case 120: simulationLoop<120>();
            break;
        default:
            ZEPHYR_ASSERT(false, "Physics ticks per second requested are not a valid value, use one of the presets: 30, 60, 90, 120");
            return;
            break;
        }

         // After exiting a simulation loop we may have requested a physics timestep change.
         // Reset this flag to not exit the next simulationLoop when looping back around this While().
        mPhysicsTimeStepChanged = false;
    }
}