#pragma once

#include "TestManager.hpp"

namespace Test
{
	class ComponentSerialiseTester : public TestManager
	{
	public:
		ComponentSerialiseTester() : TestManager(std::string("Component serialisation")) {}

	protected:
		void run_unit_tests()        override;
		void run_performance_tests() override;

	private:
		// Used only inside ComponentSerialiseTester.cpp, no need to expose it to the public.
		template<typename ComponentType>
		bool test_serialisation(const ComponentType& p_to_serialise, ComponentType& p_deserialised);
	};
} // namespace Test