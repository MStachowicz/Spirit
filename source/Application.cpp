#include "Application.hpp"

#include "Logger.hpp"
#include "JobSystem.hpp"
#include "Renderer.hpp"
#include "FileSystem.hpp"
#include "Input.hpp"
#include "Application.hpp"

 Application::Application(int argc, char *argv[])
 {
     ZEPHYR_ASSERT(argc > 0, "No arguments supplied to executable, directories cannot be initialised")

     LOG_INFO("Number of arguments passed on launch: {}", argc);
     for (int index{}; index != argc; ++index)
         LOG_INFO("Argument {}: {}", index + 1, argv[index]);

     File::setupDirectories(argv[0]);
 }

bool Application::initialise()
{
    JobSystem::initialise();

    if (!mRenderer.initialise())
        return false;

    //LOG_INFO("Zephyr initialisation took {}ms", timer.getTimeElapsedMillisecondsDouble_ms());
    return true;
}


void Application::simulationLoop()
{
    while (!Input::closeRequested())
    {
        if (usingVariablePhysicsTimestep)
            simulationLoopVariable();
        else
            simulationLoopIntegral();
    }

    LOG_INFO("------------------------------------------------------------------------");
    LOG_INFO("Total simulation time: {}s", totalTimeSeconds);
    LOG_INFO("Total physics updates: {}", mPhysicsUpdatesCount);
    LOG_INFO("Averaged physics updates per second: {}/s", physicsFPS);
    LOG_INFO("Total rendered frames: {}", mRenderer.drawCount);
    LOG_INFO("Averaged render frames per second: {}/s", renderFPS);
}


void Application::simulationLoopIntegral()
{
    LOG_INFO("Physics fixed timestep: {}ms", std::chrono::round<std::chrono::microseconds>(mIntegerPhysicsTimestep).count() / 1000.);

    IntegralDuration durationSinceLastPhysicsTick       = IntegralDuration::zero(); // Accumulated time since the last physics update.
    IntegralDuration durationSinceLastRenderTick        = IntegralDuration::zero(); // Accumulated time since the last physics update.
    IntegralDuration durationSinceLastFrame             = IntegralDuration::zero(); // Time between this frame and last frame.
    IntegralDuration durationTotalSimulation            = IntegralDuration::zero(); // Total time simulation has been running.

    IntegralTimePoint physicsTime{};            // The time point the physics is advanced to currently.
    IntegralTimePoint timeFrameStarted{};      // The time point at the start of a new frame.
    IntegralTimePoint timeLastFrameStarted  = Clock::now();

    // Continuous loop until the main window is marked for closing or Input requests close.
    // While looping the physics updates by fixed timestep mIntegerPhysicsTimestep
    // The renderer produces time and the simulation consumes it in discrete mIntegerPhysicsTimestep sized steps
    while (true)
    {
        Input::pollEvents();
        if (Input::closeRequested() || timestepChangeRequested)
            break;

        timeFrameStarted = Clock::now();
        durationSinceLastFrame = timeFrameStarted - timeLastFrameStarted;
        if (durationSinceLastFrame > maxFrameDelta)
            durationSinceLastFrame = maxFrameDelta;

        timeLastFrameStarted            = timeFrameStarted;
        durationTotalSimulation         += durationSinceLastFrame;
        durationSinceLastPhysicsTick    += durationSinceLastFrame;
        durationSinceLastRenderTick     += durationSinceLastFrame;

        // Apply physics updates until accumulated time is below mIntegerPhysicsTimestep step
        while (durationSinceLastPhysicsTick >= mIntegerPhysicsTimestep)
        {
            durationSinceLastPhysicsTick -= mIntegerPhysicsTimestep;
            physicsTime                  += mIntegerPhysicsTimestep;
            mPhysicsUpdatesCount++;

            //previousState = currentState;
            //integrate(currentState, physicsTime, mIntegerPhysicsTimestep);
            //LOG_INFO("---Physics update {}", mPhysicsUpdatesCount);
        }

        if (durationSinceLastRenderTick >= mIntegerPhysicsTimestep)
        {
            // Any remainder in the durationSinceLastPhysicsTick is a measure of how much more time is required before another physics step can be taken
            // Next interpolate between the previous and current physics state based on how much time is left in the durationSinceLastPhysicsTick
            // preventing a subtle but visually unpleasant stuttering of the physics simulation on the screen
            //LOG_INFO("---Render frame {}", mRenderedFramesCount);

            durationSinceLastRenderTick = IntegralDuration::zero();
            const double alpha = std::chrono::duration<double>{durationSinceLastPhysicsTick} / mIntegerPhysicsTimestep; // Blending factor between 0-1 used to interpolate current state
            //renderState = currentState * alpha + previousState * (1 - alpha);
            //render(renderState);
            mRenderer.draw();
        }
    }

    totalTimeSeconds    = std::chrono::round<std::chrono::milliseconds>(durationTotalSimulation).count() / 1000.;
    renderFPS           = (double)mRenderer.drawCount / totalTimeSeconds;
    physicsFPS          = (double)mPhysicsUpdatesCount / totalTimeSeconds;
}

void Application::simulationLoopVariable()
{
    VariableDuration durationSinceLastPhysicsTick   = VariableDuration::zero();
    VariableDuration durationSinceLastRenderTick    = VariableDuration::zero(); // Accumulated time since the last physics update.
    VariableDuration durationSinceLastFrame         = VariableDuration::zero();
    VariableDuration durationTotalSimulation        = VariableDuration::zero(); // Total time simulation has been running.

    VariableTimePoint  physicsTime{};
    VariableTimePoint  timeFrameStarted{};
    VariableTimePoint  timeLastFrameStarted = Clock::now();

    while (true)
    {
        Input::pollEvents();
        if (Input::closeRequested() || timestepChangeRequested)
            break;

        timeFrameStarted = Clock::now();
        durationSinceLastFrame = timeFrameStarted - timeLastFrameStarted;
        if (durationSinceLastFrame > maxFrameDelta)
            durationSinceLastFrame = maxFrameDelta;

        timeLastFrameStarted            = timeFrameStarted;
        durationTotalSimulation         += durationSinceLastFrame;
        durationSinceLastPhysicsTick    += durationSinceLastFrame;
        durationSinceLastRenderTick     += durationSinceLastFrame;

        // Apply physics updates until accumulated time is below mIntegerPhysicsTimestep step
        while (durationSinceLastPhysicsTick >= mVariablePhysicsTimestep)
        {
            durationSinceLastPhysicsTick -= mVariablePhysicsTimestep;
            physicsTime                  += mVariablePhysicsTimestep;
            mPhysicsUpdatesCount++;

            //previousState = currentState;
            //integrate(currentState, physicsTime, dt);
            //LOG_INFO("---Physics update {}", mPhysicsUpdatesCount);
        }

        if (durationSinceLastRenderTick >= mVariablePhysicsTimestep)
        {
            // Any remainder in the durationSinceLastPhysicsTick is a measure of how much more time is required before another physics step can be taken
            // Next interpolate between the previous and current physics state based on how much time is left in the durationSinceLastPhysicsTick
            // preventing a subtle but visually unpleasant stuttering of the physics simulation on the screen
            //LOG_INFO("---Render frame {}", mRenderedFramesCount);

            durationSinceLastRenderTick = VariableDuration::zero();
            const double alpha = durationSinceLastPhysicsTick / mVariablePhysicsTimestep;
            //State state = currentState * alpha + previousState * (1 - alpha);
            //render(state);
            mRenderer.draw();
        }
    }

    totalTimeSeconds   = std::chrono::round<std::chrono::milliseconds>(durationTotalSimulation).count() / 1000.;
    renderFPS          = (double)mRenderer.drawCount / totalTimeSeconds;
    physicsFPS         = (double)mPhysicsUpdatesCount / totalTimeSeconds;
}
