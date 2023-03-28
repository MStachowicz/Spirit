#pragma once

// System
#include "CollisionSystem.hpp"
#include "InputSystem.hpp"
#include "MeshSystem.hpp"
#include "PhysicsSystem.hpp"
#include "SceneSystem.hpp"
#include "TextureSystem.hpp"

// UI
#include "Editor.hpp"

// Platform
#include "Core.hpp"

// OpenGL
#include "OpenGLRenderer.hpp"

// Utility
#include "File.hpp"
#include "JobSystem.hpp"
#include "Logger.hpp"
#include "Stopwatch.hpp"

// Test
#include "TestManager.hpp"

// STD
#include <Chrono>

// Application keeps track of timing and running the simulation loop and runtime of the program.
class Application
{
public:
    Application();
    void simulationLoop();

private:
    System::TextureSystem mTextureSystem;
    System::MeshSystem mMeshSystem;
    System::SceneSystem mSceneSystem;

    OpenGL::OpenGLRenderer mOpenGLRenderer;

    System::CollisionSystem mCollisionSystem;
    System::PhysicsSystem mPhysicsSystem;
    System::InputSystem mInputSystem;

    UI::Editor mEditor;

    bool mSimulationLoopParamsChanged;       // True when the template params of simulationLoop change, causes an exit from the loop and re-run
    int mPhysicsTicksPerSecond;              // The number of physics updates to perform per second. This is equivalent to the pPhysicsTicksPerSecond template param of simulationLoop.
    int mRenderTicksPerSecond;               // The number of renders to perform per second. This is equivalent to the pRenderTicksPerSecond template param of simulationLoop.
    std::chrono::milliseconds maxFrameDelta; // If the time between loops is beyond this, cap at this duration

    // This simulation loop uses a physics timestep based on integer type giving no truncation or round-off error.
    // It's required to be templated to allow physicsTimestep to be set using std::ratio as the chrono::duration period.
    // pPhysicsTicksPerSecond: The target number of physics ticks per second. This template parameter is always equivalent to mPhysicsTicksPerSecond.
    // pRenderTicksPerSecond: The target number of renders per second. This template parameter is always equivalent to mRenderTicksPerSecond.
    template <int pPhysicsTicksPerSecond, int pRenderTicksPerSecond>
    void simulationLoop()
    {
        using Clock = std::chrono::steady_clock;

        constexpr auto physicsTimestep = std::chrono::duration<Clock::rep, std::ratio<1, pPhysicsTicksPerSecond>>{1};
        constexpr auto renderTimestep  = std::chrono::duration<Clock::rep, std::ratio<1, pRenderTicksPerSecond>>{1};

        // The resultant sum of a Clock::duration and physicsTimestep. This will be the coarsest precision that can exactly represent both a Clock::duration and 1/60 of a second.
        // Time-based arithmetic will have no truncation error, or any round-off error if Clock::duration is integral-based (std::chrono::nanoseconds for steady_clock is integral based).
        using Duration  = decltype(Clock::duration{} + physicsTimestep);
        using TimePoint = std::chrono::time_point<Clock, Duration>;

        LOG("Target physics ticks per second: {} (timestep: {} = {})", pPhysicsTicksPerSecond, std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(physicsTimestep), physicsTimestep);
        LOG("Target render ticks per second:  {} (timestep: {} = {})", pRenderTicksPerSecond, std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(renderTimestep), renderTimestep);

        Duration durationSinceLastPhysicsTick   = Duration::zero();     // Accumulated time since the last physics update.
        Duration durationSinceLastRenderTick    = Duration::zero();     // Accumulated time since the last render.
        Duration durationSinceLastFrame         = Duration::zero();     // Time between this frame and last frame.
        Duration durationApplicationRunning     = Duration::zero();     // Total time the application has been running.

        TimePoint physicsTime{};      // The time point the physics is advanced to currently.
        TimePoint timeFrameStarted{}; // The time point at the start of a new frame.
        TimePoint timeLastFrameStarted = Clock::now();

        // Continuous loop until the main window is marked for closing or Input requests close.
        // While looping the physics updates by fixed timestep physicsTimestep
        // The renderer produces time and the simulation consumes it in discrete physicsTimestep sized steps
        while (true)
        {
            Platform::Core::pollEvents();

            if (!Platform::Core::hasWindow() || mSimulationLoopParamsChanged)
                break;

            timeFrameStarted = Clock::now();
            durationSinceLastFrame = timeFrameStarted - timeLastFrameStarted;
            if (durationSinceLastFrame > maxFrameDelta)
                durationSinceLastFrame = maxFrameDelta;

            timeLastFrameStarted            = timeFrameStarted;
            durationApplicationRunning      += durationSinceLastFrame;
            durationSinceLastPhysicsTick    += durationSinceLastFrame;
            durationSinceLastRenderTick     += durationSinceLastFrame;

            // Apply physics updates until accumulated time is below physicsTimestep step
            while (durationSinceLastPhysicsTick >= physicsTimestep)
            {
                durationSinceLastPhysicsTick -= physicsTimestep;
                physicsTime                  += physicsTimestep;
                mPhysicsSystem.integrate(physicsTimestep); // PhysicsSystem::Integrate takes a floating point rep duration, conversion here is troublesome.
            }

            if (durationSinceLastRenderTick >= renderTimestep)
            {
                // Rendering is only relevant if the data changed in the physics update.
                // Not neccessary to decrement durationSinceLastRenderTick as in physics update above as repeated draws will be identical with no data changes.
                durationSinceLastRenderTick = Duration::zero();

                mOpenGLRenderer.draw();
                mEditor.draw();

                Platform::Core::swapBuffers();
            }
        }

        const auto totalTimeSeconds = std::chrono::duration_cast<std::chrono::duration<float, std::ratio<1>>>(durationApplicationRunning);
        const float renderFPS = static_cast<float>(mEditor.mDrawCount) / totalTimeSeconds.count();
        const float physicsFPS = static_cast<float>(mPhysicsSystem.mUpdateCount) / totalTimeSeconds.count();

        LOG("------------------------------------------------------------------------");
        LOG("Total simulation time: {}", totalTimeSeconds);
        LOG("Total physics updates: {}", mPhysicsSystem.mUpdateCount);
        LOG("Averaged physics updates per second: {}/s (target: {}/s)", physicsFPS, pPhysicsTicksPerSecond);
        LOG("Total rendered frames: {}", mEditor.mDrawCount);
        LOG("Averaged render frames per second: {}/s (target: {}/s)", renderFPS, pRenderTicksPerSecond);
    }
};

int main(int argc, char *argv[])
{
    Utility::Stopwatch stopwatch;

    Utility::File::setupDirectories(argv[0]);
    Platform::Core::initialise();
    JobSystem::initialise();

    Test::runUnitTests(false);

    LOG("Number of arguments passed on launch: {}", argc);
    for (int index{}; index != argc; ++index)
        LOG("Argument {}: {}", index + 1, argv[index]);

    Application app;
    LOG("Zephyr initialisation took {}", stopwatch.duration_since_start<float, std::milli>());

    app.simulationLoop();

    Platform::Core::cleanup();
    return EXIT_SUCCESS;
}