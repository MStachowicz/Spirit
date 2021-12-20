#include "Application.hpp"

#include "JobSystem.hpp"
#include "Renderer.hpp"
#include "FileSystem.hpp"
#include "Input.hpp"
#include "Application.hpp"

bool Application::initialise(int argc, char *argv[])
{
    Clock::time_point initStartTime = Clock::now();

    Logger::init(); // Logger is fully static and requires initialisation before any access.
    ZEPHYR_ASSERT(argc > 0, "No arguments supplied to executable, directories cannot be initialised")

    LOG_INFO("Number of arguments passed on launch: {}", argc);
    for (int index{}; index != argc; ++index)
        LOG_INFO("Argument {}: {}", index + 1, argv[index]);

    File::setupDirectories(argv[0]);
    JobSystem::initialise();
    if (!mRenderer.initialise())
        return false;

    LOG_INFO("Zephyr initialisation took {}ms", std::chrono::round<std::chrono::milliseconds>(Clock::now() - initStartTime).count());
    return true;
}

void Application::simulationLoop()
{
    while (!Input::closeRequested())
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