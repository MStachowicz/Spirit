#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/epsilon.hpp"
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
	struct formatter<glm::quat>
	{
		constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
	auto format(const glm::quat& q, format_context& ctx) const { return format_to(ctx.out(), "({}, {}, {}, {})", q.w, q.x, q.y, q.z); }
	};
	template<>
	struct formatter<Geometry::Triangle>
	{
		constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
		auto format(const Geometry::Triangle& v, format_context& ctx) const { return format_to(ctx.out(), "({}, {}, {})", v.m_point_1, v.m_point_2, v.m_point_3); }
	};
}

namespace Test
{
	// Modifies source_location into a IDE hyperlink friendly format
	std::string to_string(const std::source_location& p_location);

	#define CHECK_TRUE(p_conditional, p_test_name) { run_unit_test(p_conditional, p_test_name, std::format("Expected: '{}' to be true\n{}", #p_conditional, to_string(std::source_location::current()))); }
	#define CHECK_EQUAL(p_value, p_expected_value, p_test_name) { run_unit_test(p_value == p_expected_value, p_test_name, std::format("Expected {} ({}) to equal {} ({})\n{}", #p_value, p_value, #p_expected_value, p_expected_value, to_string(std::source_location::current()))); }
	#define CHECK_EQUAL_FLOAT(p_value, p_expected_value, p_test_name, p_epsilon) { run_unit_test(glm::epsilonEqual(p_value, p_expected_value, p_epsilon), p_test_name, std::format("Expected {} ({}) to equal {} ({}) with epsilon {}\n{}", #p_value, p_value, #p_expected_value, p_expected_value, p_epsilon, to_string(std::source_location::current()))); }
	#define SCOPE_SECTION(p_section_name) auto a_random_name_that_will_never_collide_with_anything = ScopeSection(p_section_name, *this);

	// A pure-virtual API for running unit tests and performance tests.
	// Override the run_unit_tests and run_performance_tests functions to add your own tests.
	// TestManagers are executed in the TestMain.cpp file.
	class TestManager
	{
	public:
		TestManager(const std::string& p_name) noexcept;

		std::string m_name;
		size_t m_unit_tests_pass_count;
		size_t m_unit_tests_fail_count;
		std::string m_unit_tests_failed_messages;

		virtual void run_unit_tests()        = 0;
		virtual void run_performance_tests() = 0;

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

		void run_unit_test(const bool& p_condition, const std::string& p_name, const std::string& p_fail_message);
		void push_section(const std::string& p_section_name);
		void pop_section();

	private:
		std::vector<size_t> section_name_lengths;
		std::string running_section_name;
	};
} // namespace Test