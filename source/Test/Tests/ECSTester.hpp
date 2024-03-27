#pragma once

#include "TestManager.hpp"

namespace Test
{
	// Wrapper for primitive types to allow adding Persistent_IDs
	template<typename T>
	class PrimitiveTypeWrapper
	{
	public:
		T value;

		constexpr PrimitiveTypeWrapper() : value{} {}
		constexpr PrimitiveTypeWrapper(const T& p_value) : value{p_value} {}
		constexpr operator T() const { return value; } // Implicitly convert a PrimitiveTypeWrapper to its underlying type.

		// operator +=
		constexpr PrimitiveTypeWrapper& operator+=(const T& p_value)
		{
			value += p_value;
			return *this;
		}
	};

	class ECSTester : public TestManager
	{
	public:
		ECSTester() : TestManager(std::string("ECS")) {}

	protected:
		void run_unit_tests()        override;
		void run_performance_tests() override;
	};
} // namespace Test