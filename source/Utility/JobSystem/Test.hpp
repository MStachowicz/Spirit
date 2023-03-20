#pragma once

#include "JobSystem.hpp"

#include <chrono>
#include <format>
#include <string>
#include <iostream>
#include "Logger.hpp"

namespace JobSystem
{
    namespace Test
    {
        struct Timer
        {
            std::string mTestName;
            std::chrono::high_resolution_clock::time_point mStartTime;

            Timer(const std::string &pTestName)
                : mTestName(pTestName), mStartTime(std::chrono::high_resolution_clock::now())
            {}

            ~Timer()
            {
                auto end = std::chrono::high_resolution_clock::now();
                LOG_INFO("{}: took {} milliseconds to complete", mTestName, std::chrono::duration_cast<std::chrono::milliseconds>(end - mStartTime).count());
            }
        };

        // Emulate a period of time in milliseconds the CPU does some work
        // pMilliseconds - milliseconds to spin this thread
        // pRecurse - if this spin() should call another spin() using JobSystem
        void spin(float pMilliseconds, bool pRecurse = false)
        {
            if (pRecurse)
                JobSystem::execute([]{ spin(100); });

            pMilliseconds /= 1000.0f;
            std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
            double ms = 0;
            while (ms < pMilliseconds)
            {
                std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
                ms = time_span.count();
            }
        }

        struct Data
        {
            float m[16] = {0};
            void compute(const float& value)
            {
                for (int i = 0; i < 16; ++i)
                    m[i] += float(value + i);
            }
        };

        // Call spin() with no parallellism as a control
        // pCount - The number of spin() calls to make
        void controlSpinTest(const size_t &pCount)
        {
            {
                auto t = Timer("Control spin test - " + std::to_string(pCount) + "x 100ms");

                for (size_t i = 0; i < pCount; i++)
                    spin(100);
            }
        }

        // Execute the same test as controlSpinTest using JobSystem::execute().
        // As long as the number of execute() calls is smaller than number of threads this should return in ~100ms
        // pCount - The number of spin() calls to make
        void parallelSpinTest(const size_t &pCount)
        {
            {
                auto t = Timer("Parallel spin test - " + std::to_string(pCount) + "x 100ms");

                for (size_t i = 0; i < pCount; i++)
                    JobSystem::execute([]{ spin(100); });

                JobSystem::wait();
            }
        }

        // Call spin function which itself will call a spin (1 layer of recursion)
        void controlRecursiveSpinTest()
        {
            auto t = Timer("Control recursive spin test - (1x 100ms) + (1x 100ms)");
            spin(100, true);
        }

        // Call a JobSystem::execute() on a spin which itself will call a JobSystem::execute() (1 layer of recursion)
        void parallelRecursiveSpinTest()
        {
            auto t = Timer("Recursive spin test - (1x 100ms) + (1x 100ms)");
            JobSystem::execute([] { spin(100, true); });
            JobSystem::wait();
        }

        // Perform some arithmetic on an object in series with no parallelism.
        // pDataSize - The size of the data array to instantiate for carrying out computation
        void controlDataTest(const size_t& pDataSize)
        {
            Data *data = new Data[pDataSize];
            {
                {
                    auto t = Timer("Control data test - data size is " + std::to_string(pDataSize));

                    for (size_t i = 0; i < pDataSize; ++i)
                        data[i].compute(i);
                }
            }

            delete[] data;
        }

        // Perform some arithmetic on an object in parallel using JobSystem::dispatch().
        void parallelDataTest(const size_t& pDataSize, const size_t& pGroupSize)
        {
            Data *data = new Data[pDataSize];
            {
                auto t = Timer("Parallel data test - data size is " + std::to_string(pDataSize) + " using group size " + std::to_string(pGroupSize));

                JobSystem::dispatch(pDataSize, pGroupSize, [&data](JobDispatchArgs args)
                                    { data[args.mJobIndex].compute(args.mJobIndex); });
                JobSystem::wait();
            }

            delete[] data;
        }

        void run()
        {
            LOG_INFO("--------------------------------------------Starting a Job System test");

            controlSpinTest(4);
            parallelSpinTest(12);

            controlRecursiveSpinTest();
            parallelRecursiveSpinTest();

            uint32_t dataCount = 1000000;
            controlDataTest(dataCount);
            parallelDataTest(dataCount, dataCount); // GroupSize = DataCount
            parallelDataTest(dataCount, 100000);
            parallelDataTest(dataCount, 10000);
            parallelDataTest(dataCount, 1000);
            parallelDataTest(dataCount, 100);
            parallelDataTest(dataCount, 10);
            parallelDataTest(dataCount, 1);

            LOG_INFO("--------------------------------------------Job System test complete");
        }
    }
}