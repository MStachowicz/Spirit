#pragma once

#include "TestManager.hpp"

namespace Test
{
    class ResourceManagerTester : public TestManager
    {
    public:
        ResourceManagerTester() : TestManager(std::string("RESOURCE MANAGER")) {}

        void runUnitTests()        override;
        void runPerformanceTests() override;
    };
} // namespace Test