#pragma once

#include "UnitTest.hpp"

namespace ECS
{
    class Storage;
}

namespace Test
{
    class ECSUnitTester : public UnitTest
    {
    public:
        ECSUnitTester() : UnitTest(std::string("ECS")) {}
        void runAllTests() override;
    private:
        size_t countEntities(ECS::Storage& pStorage);
        void runMemoryTests(const std::string& pTestName, const size_t& pAliveCountExpected);
    };
} // namespace Test