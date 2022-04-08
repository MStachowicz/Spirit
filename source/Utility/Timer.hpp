#pragma once

// Credit:
// https://codereview.stackexchange.com/questions/196245/extremely-simple-timer-class-in-c

#include <chrono>
#include <optional>

class TimerContainer;

// Stopwatch retuning elapsed time in any period and data type requested.
template <typename Clock = std::chrono::high_resolution_clock>
class Stopwatch
{
public:
    // Creates and starts a stopwatch.
    Stopwatch() : start_point(Clock::now()) {}

    // Returns the time elapsed since StopWatch construction.
    // Period: The std::ratio used to cast the elapsed time relative to seconds. E.g. std::milli for milliseconds
    // Type: The type to cast the duration to. e.g. float (allows fractions of Period e.g. 16.667)
    template <typename Period, typename Type>
    Type getTime() const
    {
        auto duration       = Clock::now() - start_point;
        auto castDuration   = std::chrono::duration_cast<std::chrono::duration<Type, Period>>(duration);
        auto counted_time   = castDuration.count();
        return static_cast<Type>(counted_time);
    }

private:
    const typename Clock::time_point start_point;
};

using SystemStopwatch       = Stopwatch<std::chrono::system_clock>;
using MonotonicStopwatch    = Stopwatch<std::chrono::steady_clock>;