#pragma once

#include "TestManager.hpp"

namespace Test
{
	class ResourceManagerTester : public TestManager
	{
	public:
		ResourceManagerTester() : TestManager(std::string("RESOURCE MANAGER")) {}

		void run_unit_tests()        override;
		void run_performance_tests() override;
	};
} // namespace Test