#pragma once

#include "TestManager.hpp"

namespace ECS
{
    class Storage;
}

namespace Test
{
    class ECSTester : public TestManager
    {
    public:
        ECSTester() : TestManager(std::string("ECS")) {}

    protected:
        void runUnitTests()        override;
        void runPerformanceTests() override;

    private:
        size_t countEntities(ECS::Storage& pStorage);
        void runMemoryTests(const std::string& pTestName, const size_t& pAliveCountExpected);
    };
} // namespace Test