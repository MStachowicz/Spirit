#pragma once

#include "Stopwatch.hpp"

#include "glm/glm.hpp"

#include <string>
#include <vector>
#include <format>
#include <source_location>

namespace std
{
    template<>
    struct formatter<glm::vec3> {
        template<typename ParseContext>
        constexpr auto parse(ParseContext& ctx) {
            return ctx.begin();
        }

        template<typename FormatContext>
        auto format(const glm::vec3& v, FormatContext& ctx) {
            return format_to(ctx.out(), "({}, {}, {})", v.x, v.y, v.z);
        }
    };
}

namespace Test
{
    // Modifies source_location into a IDE hyperlink friendly format
    std::string to_string(const std::source_location& p_location);

    #define CHECK_TRUE(p_conditional, p_test_name)        { runUnitTest({p_conditional, p_test_name,         std::format("Expected: '{}' to be true\n{}", #p_conditional,        to_string(std::source_location::current()))}); }
    #define CHECK_EQUAL(p_value, p_expected_value, p_test_name) { runUnitTest({p_value == p_expected_value, p_test_name, std::format("Expected: '{}' to be '{}' but was '{}'\n{}", #p_value, #p_expected_value, p_value, to_string(std::source_location::current()))}); }
    //TODO: ADD CHECK_THROW (make sure the conditional or func throws and exception)

    using TestDuration = std::chrono::duration<float, std::milli>;

    void runUnitTests(const bool& pRunPerformanceTests);

    // Represents a single unit test.
    class UnitTest
    {
    public:
        UnitTest(const bool& pCondition, const std::string& pName, const std::string& pFailMessage) noexcept;
        bool mPassed;       // Did the test pass
        std::string mName;  // Title of the test
        std::string mFailMessage; // Message outputted if the test fails
    };
    // Represents the mTimeTaken to execute the Func passed as the constructor parameter pTestFunc.
    class PerformanceTest
    {
        static constexpr size_t RepeatCount = 10; // Number of times the test will be repeated to find a more accurate average time taken.
    public:
        std::string mName;  // Title of the test
        TestDuration mTimeTaken;

        template<typename Func>
        PerformanceTest(const std::string& pName, Func& pTestFunc) noexcept
            : mName{pName}
            , mTimeTaken{}
        {
            Utility::Stopwatch stopwatch;
            for (size_t i = 0; i < RepeatCount; i++)
                pTestFunc();
            mTimeTaken = stopwatch.duration_since_start<float, std::milli>() / RepeatCount; // Divide to find the average time taken.
        }
    };
    // A pure-virtual API for running unit tests and performance tests.
    // Override the runUnitTests and runPerformanceTests functions to add your own tests.
    // When run() is called, the results of all the tests will be outputted.
    class TestManager
    {
        std::string mName;

        size_t mUnitTestsPassed;
        size_t mUnitTestsFailed;
        std::vector<UnitTest> mUnitTests;
        TestDuration mTimeTakenUnitTests; // Time taken to complete all the unit tests

        std::vector<PerformanceTest> mPerformanceTests;
        TestDuration mTimeTakenPerformanceTests; // Time taken to complete all the performance tests

    protected:
        // Pushes the test to mUnitTests and updates the running totals.
        void runUnitTest(const UnitTest&& pTest);
        // Pushes the test to mPerformance tests and updates the running totals.
        void runPerformanceTest(const PerformanceTest&& pTest);

        virtual void runUnitTests()        = 0;
        virtual void runPerformanceTests() = 0;

    public:
        TestManager(const std::string& pName) noexcept;

        // Executes all the tests filling up the mUnitTests and mPerformanceTests containers.
        // When completed, outputs the results of all the tests.
        void run(const bool& pRunPerformanceTests);
    };
} // namespace Test