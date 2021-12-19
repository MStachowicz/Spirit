#pragma once

#include "Chrono"
#include "Renderer.hpp"

// Application keeps track of timing and running the simulation loop and runtime of the program.
class Application
{
public:
    Application(int argc, char *argv[]);

    bool initialise();
    void simulationLoop();

private:
    Renderer mRenderer;


    // Runs a simulation loop using std:ratio as phsycics timestep, this avoids and truncation or rounding errors in the regular
    // double based duration for timestep by using integral based time units.
    void simulationLoopIntegral();
    // Regular simulation loop using double based duration, prone to rounding and truncation errors but allows a variable physics timestep.
    void simulationLoopVariable();


    const std::chrono::milliseconds maxFrameDelta = std::chrono::milliseconds(250); // If the time between loops is beyond this, cap at this duration
    bool timestepChangeRequested = false;                                           // True when the physics timestep is changed, causes an exit from the loop and re-run
    double totalTimeSeconds = 0.0;
    double renderFPS = 0.0;
    double physicsFPS = 0.0;
    int mPhysicsUpdatesCount = 0;

    typedef std::chrono::steady_clock Clock;


    // The following definitions and members are used when usingVariablePhysicsTimestep is true.
    // -----------------------------------------------------------------------------------------------------
    bool usingVariablePhysicsTimestep = true;
    const double physicsUpdatesPerSecond = 60;
    typedef std::chrono::duration<double> VariableDuration;
    typedef std::chrono::time_point<Clock, VariableDuration> VariableTimePoint;
    VariableDuration mVariablePhysicsTimestep = std::chrono::seconds(1) / physicsUpdatesPerSecond;

    // The following definitionsare used when usingVariablePhysicsTimestep is false.
    // -----------------------------------------------------------------------------------------------------
    // physicsTimestep has the value 1, represented by a long long, and has units of 1/60 of a second. This is set at compile time so cannot change
    const std::chrono::duration<long long, std::ratio<1, 60>> mIntegerPhysicsTimestep = std::chrono::duration<long long, std::ratio<1, 60>>{1};
    // The resultant sum of a Clock::duration and mIntegerPhysicsTimestep. This will be the coarsest precision that can exactly represent both
    // a Clock::duration and 1/60 of a second. Time-based arithmetic will have no truncation error, or any round-off error
    // if Clock::duration is integral-based (std::chrono::nanoseconds for steady_clock is integral based).
    typedef decltype(Clock::duration{} + mIntegerPhysicsTimestep)    IntegralDuration;
    typedef std::chrono::time_point<Clock, IntegralDuration>         IntegralTimePoint;
};
