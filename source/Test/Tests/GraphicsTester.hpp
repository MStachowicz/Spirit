#pragma once

#include "Test/TestManager.hpp"

namespace Test
{
	class GraphicsTester : public TestManager
	{
	public:
		GraphicsTester() : TestManager(std::string("GRAPHICS")) {}

		void run_unit_tests()        override;
		void run_performance_tests() override;
	};
} // namespace Test