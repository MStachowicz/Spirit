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
#include "Window.hpp"
#include "Input.hpp"

// OpenGL
#include "OpenGLRenderer.hpp"
#include "DebugRenderer.hpp"
#include "OpenGL/GridRenderer.hpp"

// Utility
#include "File.hpp"
#include "Logger.hpp"
#include "Stopwatch.hpp"

// Test
#include "TestManager.hpp"

// STD
#include <chrono>

// Application manages the ownership and calling of all the Systems.
// Taking an OS window it renders and updates the state of an ECS.
class Application
{
public:
    Application(Platform::Input& p_input, Platform::Window& p_window) noexcept;
    ~Application() noexcept;
    void simulationLoop();

private:
    Platform::Input& m_input;
    Platform::Window& m_window; // Main window all application business takes place in. When this window is closed, the application ends and vice-versa.

    System::TextureSystem mTextureSystem;
    System::MeshSystem mMeshSystem;
    System::SceneSystem mSceneSystem;

    OpenGL::OpenGLRenderer mOpenGLRenderer;
    OpenGL::GridRenderer m_grid_renderer;

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

        LOG("Target physics ticks per second: {} (timestep: {}ms = {})", pPhysicsTicksPerSecond, std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(physicsTimestep).count(), physicsTimestep);
        LOG("Target render ticks per second:  {} (timestep: {}ms = {})", pRenderTicksPerSecond, std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(renderTimestep).count(), renderTimestep);

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
            OpenGL::DebugRenderer::clear();

            m_input.update(); // Poll events then check close_requested.
            if (m_window.close_requested() || mSimulationLoopParamsChanged)
                break;

            mInputSystem.update();

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
                m_window.start_ImGui_frame();
                mOpenGLRenderer.start_frame();

                m_grid_renderer.draw();
                mOpenGLRenderer.draw();
                mEditor.draw(durationSinceLastRenderTick);
                OpenGL::DebugRenderer::render(mSceneSystem);

                mOpenGLRenderer.end_frame();
                m_window.end_ImGui_frame();
                m_window.swap_buffers();

                durationSinceLastRenderTick = Duration::zero();
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
    {
        Utility::Stopwatch stopwatch;

        // Library init order is important here
        // GLFW <- Window/GL context <- OpenGL functions <- ImGui <- App
        Platform::Core::initialise_directories();
        Platform::Core::initialise_GLFW();
        Platform::Input input   = Platform::Input();
        Platform::Window window = Platform::Window(1920, 1080, input);
        Platform::Core::initialise_OpenGL();
        OpenGL::DebugRenderer::init();
        Platform::Core::initialise_ImGui(window);
        //Test::runUnitTests(false);

        LOG("[INIT] Number of arguments passed on launch: {}", argc);
        for (int index{}; index != argc; ++index)
            LOG("Argument {}: {}", index + 1, argv[index]);

        auto app = Application(input, window);
        LOG("[INIT] initialisation took {}", stopwatch.duration_since_start<int, std::milli>());

        app.simulationLoop();
    } // Window and input must go out of scope and destroy their resources before Core::cleanup

    Platform::Core::cleanup();
    OpenGL::DebugRenderer::deinit();
    return EXIT_SUCCESS;
}