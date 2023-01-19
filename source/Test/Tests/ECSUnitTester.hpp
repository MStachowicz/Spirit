#pragma once

#include "UnitTest.hpp"

namespace Test
{
    class ECSUnitTester : public UnitTest
    {
    public:
        ECSUnitTester() : UnitTest(std::string("ECS")) {}
        void runAllTests() override;
    };
} // namespace Test