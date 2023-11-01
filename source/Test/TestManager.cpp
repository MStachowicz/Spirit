#include "TestManager.hpp"
#include "ECSTester.hpp"
#include "GeometryTester.hpp"
#include "ResourceManagerTester.hpp"

#include "Logger.hpp"
#include "Stopwatch.hpp"

#include <format>

namespace Test
{
    static const std::string seperator = "*****************************************************************\n";

    std::string to_string(const std::source_location& p_location)
    {
        return std::format("{}:{}", p_location.file_name(), p_location.line());
    }

    void runUnitTests(const bool& pRunPerformanceTests)
    {
        LOG("{} Starting Unit tests", seperator);
        Utility::Stopwatch stopwatch;

        ECSTester tester;
        tester.run(pRunPerformanceTests);

        GeometryTester geometryTester;
        geometryTester.run(pRunPerformanceTests);

        ResourceManagerTester resource_manager_tester;
        resource_manager_tester.run(pRunPerformanceTests);

        LOG("All Unit tests complete - Time taken: {}ms\n{}", stopwatch.duration_since_start<float, std::milli>().count(), seperator);
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
                    output += std::format("PASSED '{}'\n", test.mName);
                else
                    output += std::format("FAILED '{}' -> {}\n", test.mName, test.mFailMessage);
            }
            for (const auto& test : mPerformanceTests)
                output += std::format("PERF TEST '{}' - TOOK {}ms\n", test.mName, test.mTimeTaken.count());

            output += std::format("***************** {} TEST SUMMARY *****************\n", mName);
            output += std::format("----------------- UNIT TESTS -----------------\n");
            output += std::format("TOTAL TESTS: {}\nPASSED: {}\nFAILED: {}\nTIME TAKEN: {}ms\n", mUnitTests.size(), mUnitTestsPassed, mUnitTestsFailed, mTimeTakenUnitTests.count());

            if (pRunPerformanceTests)
            {
                output += std::format("----------------- PERFORMANCE TESTS -----------------\n");
                output += std::format("TOTAL TESTS: {}\nTIME TAKEN: {}ms\n", mPerformanceTests.size(), mTimeTakenPerformanceTests.count());
            }
            output += seperator;
        }

        LOG("{}", output);
    }

} // namespace Test