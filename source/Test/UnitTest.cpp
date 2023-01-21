#include "UnitTest.hpp"
#include "ECSUnitTester.hpp"
#include "GeometryTester.hpp"

#include "Logger.hpp"
#include "Stopwatch.hpp"

#include <format>

namespace Test
{
    static const std::string seperator = "*****************************************************************\n";

    void runUnitTests()
    {
        LOG_INFO("{} Starting Unit tests", seperator);
        Utility::Stopwatch stopwatch;

        ECSUnitTester tester;
        tester.run();

        GeometryTester geometryTester;
        geometryTester.run();

        LOG_INFO("{} All Unit tests complete - Time taken: {}ms\n{}{}", seperator, stopwatch.getTime<std::milli, float>(), seperator, seperator);
    }

    Test::Test(const bool& pCondition, const std::string& pName, const std::string& pFailMessage)
        : mPassed{pCondition}
        , mName{pName}
        , mFailMessage{pFailMessage}
    {}
    UnitTest::UnitTest(const std::string& pName)
        : mName{pName}
        , mPassed{0}
        , mFailed{0}
        , mTests{}
    {}

    void UnitTest::runTest(const Test&& pTest)
    {
        mTests.emplace_back(std::forward<const Test>(pTest));
        if (mTests.back().mPassed)
            mPassed++;
        else
            mFailed++;
    }

    void UnitTest::run()
    {
        Utility::Stopwatch stopwatch;
        std::string output = seperator + std::format("\n----------------- {} UNIT TEST STARTING -----------------\n", mName);

        runAllTests(); // Run all the tests and fill mTests here.

        output.reserve(mTests.size() * (60)); // assume number of chars per test output average
        for (const auto& test : mTests)
        {
            if (test.mPassed)
                output += std::format("TEST '{}' - PASSED\n", test.mName);
            else
                output += std::format("TEST '{}' - FAILED - {}\n", test.mName, test.mFailMessage);
        }

        output += std::format("----------------- {} UNIT TEST SUMMARY ({}) -----------------\n", mName, mFailed == 0 ? "PASSED" : "FAILED");

        output += std::format("PASSED: {}\nFAILED: {}\nTOTAL TESTS: {}\nTIME TAKEN: {}ms\n", mPassed, mFailed, mPassed + mFailed, stopwatch.getTime<std::milli, float>());
        output += seperator;

        LOG_INFO(output);
    }

} // namespace Test