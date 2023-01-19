#pragma once

#include <string>
#include <vector>

namespace Test
{
    void runUnitTests();

    // Represents a single unit test.
    class Test
    {
    public:
        Test(const bool& pCondition, const std::string& pName, const std::string& pFailMessage);
        bool mPassed;       // Did the test pass
        std::string mName;  // Title of the test
        std::string mFailMessage; // Message outputted if the test fails
    };

    // A collection of tests and API for running them and outputting the results
    class UnitTest
    {
        std::string mName;
        size_t mPassed;
        size_t mFailed;
        std::vector<Test> mTests;

    protected:
        // Push a test to mTests
        void runTest(const Test&& pTest);
        // Run all the tests and fill up the mTests container
        virtual void runAllTests() = 0;
    public:
        UnitTest(const std::string& pName);
        void run();
    };
} // namespace Test