#pragma once

#include "TestManager.hpp"

namespace Test
{
	class QuadTreeTester : public TestManager
	{
	public:
		QuadTreeTester() : TestManager(std::string("QUAD TREE TEST")) {}

		void run_unit_tests()        override;
		void run_performance_tests() override {};
	};
} // namespace Test