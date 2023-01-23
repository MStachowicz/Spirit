#pragma once

#include "UnitTest.hpp"

namespace Test
{
    class GeometryTester : public UnitTest
    {
    public:
        GeometryTester() : UnitTest(std::string("Geometry")) {}
        void runAllTests() override;

    private:
        void runAABBTests();
        void runTriangleTests();
    };
} // namespace Test