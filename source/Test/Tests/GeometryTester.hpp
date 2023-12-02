#pragma once

#include "TestManager.hpp"

namespace Test
{
	class GeometryTester : public TestManager
	{
	public:
		GeometryTester() : TestManager(std::string("GEOMETRY")) {}

		void run_unit_tests()        override;
		void run_performance_tests() override;

		static void draw_frustrum_debugger_UI(float aspect_ratio);
	private:
		void runAABBTests();
		void runTriangleTests();
		void run_frustrum_tests();
		void run_sphere_tests();
		void run_point_tests();
	};
} // namespace Test