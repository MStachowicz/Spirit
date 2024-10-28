#pragma once

#include "Test/TestManager.hpp"

namespace Test
{
	class CameraTester : public TestManager
	{
	public:
		CameraTester() : TestManager(std::string("CAMERA")) {}

		void run_unit_tests()        override;
		void run_performance_tests() override {}
	};
} // namespace Test