#include "UnitTest.hpp"
#include "ECSUnitTester.hpp"

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

        LOG_INFO("{} All Unit tests complete - Time taken: {}\n{}{}", seperator, stopwatch.getTime<std::milli, float>(), seperator, seperator);
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
        std::string output = seperator + std::format("{} UNIT TEST STARTING", mName);

        runAllTests(); // Run all the tests and fill mTests here.

        output.reserve(mTests.size() * (60)); // assume number of chars per test output average
        for (const auto& test : mTests)
        {
            if (test.mPassed)
                output += std::format("TEST: {} PASSED", test.mName);
            else
                output += std::format("TEST: {} FAILED - {}", test.mName, test.mFailMessage);
        }

        output += seperator + std::format("{} UNIT TEST SUMMARY", mName);
        if (mFailed == 0)
            output += std::format("{} UNIT TEST PASSED\nPASSED: {}\nFAILED: {}\nTOTAL TESTS: {}\nTIME TAKEN: {}", mName, mPassed, mFailed, mPassed + mFailed, stopwatch.getTime<std::milli, float>());
        else
            output += std::format("{} UNIT TEST FAILED\nPASSED: {}\nFAILED: {}\nTOTAL TESTS: {}\nTIME TAKEN: {}", mName, mPassed, mFailed, mPassed + mFailed, stopwatch.getTime<std::milli, float>());
        output += seperator + seperator;

        LOG_INFO(output);
    }

} // namespace Test