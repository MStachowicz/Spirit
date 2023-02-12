#pragma once

#include "TestManager.hpp"

namespace Test
{
    class GeometryTester : public TestManager
    {
    public:
        GeometryTester() : TestManager(std::string("GEOMETRY")) {}

        void runUnitTests()        override;
        void runPerformanceTests() override;

    private:
        void runAABBTests();
        void runTriangleTests();
    };
} // namespace Test