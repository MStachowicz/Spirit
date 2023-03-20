#include "TestManager.hpp"
#include "ECSTester.hpp"
#include "GeometryTester.hpp"

#include "Logger.hpp"
#include "Stopwatch.hpp"

#include <format>

namespace Test
{
    static const std::string seperator = "*****************************************************************\n";

    void runUnitTests(const bool& pRunPerformanceTests)
    {
        LOG("{} Starting Unit tests", seperator);
        Utility::Stopwatch stopwatch;

        ECSTester tester;
        tester.run(pRunPerformanceTests);

        GeometryTester geometryTester;
        geometryTester.run(pRunPerformanceTests);

        LOG("All Unit tests complete - Time taken: {}ms\n{}{}", stopwatch.getTime<std::milli, float>(), seperator);
    }

    UnitTest::UnitTest(const bool& pCondition, const std::string& pName, const std::string& pFailMessage) noexcept
        : mPassed{pCondition}
        , mName{pName}
        , mFailMessage{pFailMessage}
    {}
    TestManager::TestManager(const std::string& pName) noexcept
        : mName{pName}
        , mUnitTestsPassed{0}
        , mUnitTestsFailed{0}
        , mUnitTests{}
        , mTimeTakenUnitTests{}
        , mPerformanceTests{}
        , mTimeTakenPerformanceTests{}
    {}

    void TestManager::runUnitTest(const UnitTest&& pTest)
    {
        mUnitTests.emplace_back(std::forward<const UnitTest>(pTest));
        if (mUnitTests.back().mPassed)
            mUnitTestsPassed++;
        else
            mUnitTestsFailed++;
    }
    void TestManager::runPerformanceTest(const PerformanceTest&& pTest)
    {
        mPerformanceTests.emplace_back(std::forward<const PerformanceTest>(pTest));
    }

    void TestManager::run(const bool& pRunPerformanceTests)
    {
        { // Unit Tests
            Utility::Stopwatch stopwatch;
            runUnitTests();
            mTimeTakenUnitTests = stopwatch.duration_since_start<float, std::milli>();
        }
        if (pRunPerformanceTests)
        {
            Utility::Stopwatch stopwatch;
            runPerformanceTests();
            mTimeTakenPerformanceTests = stopwatch.duration_since_start<float, std::milli>();
        }

        std::string output = seperator + std::format("\n----------------- {} TESTS STARTING -----------------\n", mName);
        output.reserve((mUnitTests.size() + mPerformanceTests.size()) * (60)); // Assuming an average of 60 chars per test summary here
        { // Build the output string
            for (const auto& test : mUnitTests)
            {
                if (test.mPassed)
                    output += std::format("UNIT TEST '{}' - PASSED\n", test.mName);
                else
                    output += std::format("UNIT TEST '{}' - FAILED - {}\n", test.mName, test.mFailMessage);
            }
            for (const auto& test : mPerformanceTests)
                output += std::format("PERF TEST '{}' - TOOK {}\n", test.mName, test.mTimeTaken);

            output += std::format("***************** {} TEST SUMMARY *****************\n", mName);
            output += std::format("----------------- UNIT TESTS -----------------\n");
            output += std::format("TOTAL TESTS: {}\nPASSED: {}\nFAILED: {}\nTIME TAKEN: {}\n", mUnitTests.size(), mUnitTestsPassed, mUnitTestsFailed, mTimeTakenUnitTests);

            if (pRunPerformanceTests)
            {
                output += std::format("----------------- PERFORMANCE TESTS -----------------\n");
                output += std::format("TOTAL TESTS: {}\nTIME TAKEN: {}\n", mPerformanceTests.size(), mTimeTakenPerformanceTests);
            }
            output += seperator;
        }


        LOG(output);
    }

} // namespace Test