#pragma once

#include "Stopwatch.hpp"

#include "glm/glm.hpp"
#include "Geometry/Point.hpp"
#include "Geometry/Triangle.hpp"

#include <string>
#include <vector>
#include <format>
#include <source_location>
#include <optional>

namespace std
{
	template<>
	struct formatter<glm::vec3>
	{
		constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
		auto format(const glm::vec3& v, format_context& ctx) const { return format_to(ctx.out(), "({}, {}, {})", v.x, v.y, v.z); }
	};
	template<>
	struct formatter<Geometry::Point>
	{
		constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
		auto format(const Geometry::Point& v, format_context& ctx) const { return format_to(ctx.out(), "({}, {}, {})", v.m_position.x, v.m_position.y, v.m_position.z); }
	};
	template<>
	struct formatter<std::optional<Geometry::Point>>
	{
		constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
		auto format(const std::optional<Geometry::Point>& v, format_context& ctx) const { return v.has_value() ? format_to(ctx.out(), "({}, {}, {})", v->m_position.x, v->m_position.y, v->m_position.z) : format_to(ctx.out(), "nullopt"); }
	};
	template<>
	struct formatter<Geometry::Triangle>
	{
		constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
		auto format(const Geometry::Triangle& v, format_context& ctx) { return format_to(ctx.out(), "({}, {}, {})", v.m_point_1, v.m_point_2, v.m_point_3); }
	};
}

namespace Test
{
	// Modifies source_location into a IDE hyperlink friendly format
	std::string to_string(const std::source_location& p_location);

	#define CHECK_TRUE(p_conditional, p_test_name) { runUnitTest({p_conditional, p_test_name,         std::format("Expected: '{}' to be true\n{}", #p_conditional,        to_string(std::source_location::current()))}); }
	#define CHECK_EQUAL(p_value, p_expected_value, p_test_name) { runUnitTest({p_value == p_expected_value, p_test_name, std::format("Expected: '{}' to be '{}' but was '{}'\n{}", #p_value, #p_expected_value, p_value, to_string(std::source_location::current()))}); }
	#define SCOPE_SECTION(p_section_name) auto a_random_name_that_will_never_collide_with_anything = ScopeSection(p_section_name, *this);

	using TestDuration = std::chrono::duration<float, std::milli>;

	void runUnitTests(const bool& pRunPerformanceTests);

	// A pure-virtual API for running unit tests and performance tests.
	// Override the runUnitTests and runPerformanceTests functions to add your own tests.
	// When run() is called, the results of all the tests will be outputted.
	class TestManager
	{
	protected:
		// Represents a section of tests pushing and popping the p_section_name to the TestManager running_section_name
		// on construction and destruction respectively via push_section and pop_section.
		struct ScopeSection
		{
			TestManager& test_manager;

			ScopeSection(const std::string& p_section_name, TestManager& test_manager) noexcept
				: test_manager{test_manager}
			{
				test_manager.push_section(p_section_name);
			}
			~ScopeSection() noexcept
			{
				test_manager.pop_section();
			}
		};
		friend ScopeSection;
		// Represents a single unit test. Pushed to the TestManager using runUnitTest via CHECK_TRUE and CHECK_EQUAL macros.
		struct UnitTest
		{
			UnitTest(const bool& pCondition, const std::string& pName, const std::string& pFailMessage) noexcept;
			bool mPassed;       // Did the test pass
			std::string mName;  // Title of the test
			std::string mFailMessage; // Message outputted if the test fails
		};
		// Represents the mTimeTaken to execute the Func passed as the constructor parameter pTestFunc.
		struct PerformanceTest
		{
			static constexpr size_t RepeatCount = 10; // Number of times the test will be repeated to find a more accurate average time taken.

			std::string mName;  // Title of the test
			TestDuration mTimeTaken;

			template<typename Func>
			PerformanceTest(const std::string& pName, Func& pTestFunc) noexcept
				: mName{pName}
				, mTimeTaken{}
			{
				Utility::Stopwatch stopwatch;
				for (size_t i = 0; i < RepeatCount; i++)
					pTestFunc();
				mTimeTaken = stopwatch.duration_since_start<float, std::milli>() / RepeatCount; // Divide to find the average time taken.
			}
		};


		// Pushes the test to mUnitTests and updates the running totals.
		void runUnitTest(UnitTest&& pTest);
		// Pushes the test to mPerformance tests and updates the running totals.
		void runPerformanceTest(const PerformanceTest&& pTest);

		void push_section(const std::string& p_section_name);
		void pop_section();
		virtual void runUnitTests()        = 0;
		virtual void runPerformanceTests() = 0;

	public:
		TestManager(const std::string& pName) noexcept;

		// Executes all the tests filling up the mUnitTests and mPerformanceTests containers.
		// When completed, outputs the results of all the tests.
		void run(const bool& pRunPerformanceTests);

	private:
		std::string mName;
		std::vector<size_t> section_name_lengths;
		std::string running_section_name;

		size_t mUnitTestsPassed;
		size_t mUnitTestsFailed;
		std::vector<UnitTest> mUnitTests;
		TestDuration mTimeTakenUnitTests; // Time taken to complete all the unit tests

		std::vector<PerformanceTest> mPerformanceTests;
		TestDuration mTimeTakenPerformanceTests; // Time taken to complete all the performance tests
	};
} // namespace Test