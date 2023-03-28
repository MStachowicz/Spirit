#pragma once

// Credit:
// https://codereview.stackexchange.com/questions/196245/extremely-simple-timer-class-in-c

#include <chrono>

namespace Utility
{
    // Stopwatch retuning elapsed time in any period and data type requested.
    template <typename Clock = std::chrono::high_resolution_clock>
    class Stopwatch
    {
    public:
        // Creates AND starts a stopwatch. Call getTime to return the time interval from this start point.
        Stopwatch() : start_point(Clock::now())
        {}

        // Returns the time elapsed since StopWatch construction.
        // Period: The std::ratio used to cast the elapsed time relative to seconds. E.g. std::milli for milliseconds
        // Type: The type to cast the duration to. e.g. float (allows fractions of Period e.g. 16.667)
        template <typename Period, typename Type>
        Type getTime() const
        {
            auto duration     = Clock::now() - start_point;
            auto castDuration = std::chrono::duration_cast<std::chrono::duration<Type, Period>>(duration);
            auto counted_time = castDuration.count();
            return static_cast<Type>(counted_time);
        }

        // Returns the time elapsed since StopWatch construction.
        // Type: The type to cast the duration to. e.g. float (allows fractions of Period e.g. 16.667)
        // Period: The std::ratio used to cast the elapsed time relative to seconds. E.g. std::milli for milliseconds
        template <typename Type, typename Period>
        std::chrono::duration<Type, Period> duration_since_start() const
        {
            return std::chrono::duration_cast<std::chrono::duration<Type, Period>>(Clock::now() - start_point);
        }

    private:
        const typename Clock::time_point start_point;
    };
} // namespace Utility