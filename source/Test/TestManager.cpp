#include "TestManager.hpp"
#include "ECSTester.hpp"
#include "GeometryTester.hpp"
#include "ResourceManagerTester.hpp"

#include "Utility/Logger.hpp"
#include "Utility/Stopwatch.hpp"

#include <format>

namespace Test
{
	const std::string seperator = "*****************************************************************\n";

	std::string to_string(const std::source_location& p_location)
	{
		return std::format("{}:{}", p_location.file_name(), p_location.line());
	}

	std::pair<size_t, size_t> run_unit_tests(const bool& pRunPerformanceTests)
	{
		LOG("{} Starting Unit tests", seperator);
		Utility::Stopwatch stopwatch;

		ECSTester ecs_tester;
		ecs_tester.run(pRunPerformanceTests);

		GeometryTester geometry_tester;
		geometry_tester.run(pRunPerformanceTests);

		ResourceManagerTester resource_manager_tester;
		resource_manager_tester.run(pRunPerformanceTests);

		LOG("All Unit tests complete - Time taken: {}ms\n{}", stopwatch.duration_since_start<float, std::milli>().count(), seperator);

		auto total_passes = ecs_tester.get_test_pass_count() +
		                    geometry_tester.get_test_pass_count() +
		                    resource_manager_tester.get_test_pass_count();
		auto total_fails = ecs_tester.get_test_fail_count() +
		                   geometry_tester.get_test_fail_count() +
		                   resource_manager_tester.get_test_fail_count();
		return { total_passes, total_fails };
	}

	TestManager::UnitTest::UnitTest(const bool& pCondition, const std::string& pName, const std::string& pFailMessage) noexcept
		: mPassed{pCondition}
		, mName{pName}
		, mFailMessage{pFailMessage}
	{}
	TestManager::TestManager(const std::string& pName) noexcept
		: mName{pName}
		, section_name_lengths{}
		, running_section_name{""}
		, m_unit_tests_pass_count{0}
		, m_unit_tests_fail_count{0}
		, m_unit_tests{}
		, m_unit_tests_time_taken{}
		, m_performance_tests{}
		, m_performance_tests_time_taken{}
	{}

	void TestManager::emplace_unit_test(UnitTest&& pTest)
	{
		if (!running_section_name.empty())
			pTest.mName = running_section_name + " - " + pTest.mName;

		m_unit_tests.emplace_back(std::forward<const UnitTest>(pTest));

		if (m_unit_tests.back().mPassed)
			m_unit_tests_pass_count++;
		else
			m_unit_tests_fail_count++;
	}
	void TestManager::push_section(const std::string& p_section_name)
	{
		running_section_name += '[' + p_section_name + ']';
		section_name_lengths.push_back(p_section_name.length() + 2); // +2 for the [] around the section name
	}
	void TestManager::pop_section()
	{
		running_section_name = running_section_name.substr(0, running_section_name.length() - section_name_lengths.back());
		section_name_lengths.pop_back();
	}
	void TestManager::emplace_performance_test(const PerformanceTest&& pTest)
	{
		m_performance_tests.emplace_back(std::forward<const PerformanceTest>(pTest));
	}

	void TestManager::run(const bool& pRunPerformanceTests)
	{
		{ // Unit Tests
			Utility::Stopwatch stopwatch;
			run_unit_tests();
			m_unit_tests_time_taken = stopwatch.duration_since_start<float, std::milli>();
		}
		if (pRunPerformanceTests)
		{
			Utility::Stopwatch stopwatch;
			run_performance_tests();
			m_performance_tests_time_taken = stopwatch.duration_since_start<float, std::milli>();
		}

		std::string output = seperator + std::format("\n----------------- {} TESTS STARTING -----------------\n", mName);
		output.reserve((m_unit_tests.size() + m_performance_tests.size()) * (60)); // Assuming an average of 60 chars per test summary here
		{ // Build the output string
			for (const auto& test : m_unit_tests)
			{
				if (test.mPassed)
					output += std::format("PASSED '{}'\n", test.mName);
				else
					output += std::format("FAILED '{}' -> {}\n", test.mName, test.mFailMessage);
			}
			for (const auto& test : m_performance_tests)
				output += std::format("PERF TEST '{}' - TOOK {}ms\n", test.mName, test.mTimeTaken.count());

			output += std::format("***************** {} TEST SUMMARY *****************\n", mName);
			output += std::format("----------------- UNIT TESTS -----------------\n");
			output += std::format("TOTAL TESTS: {}\nPASSED: {}\nFAILED: {}\nTIME TAKEN: {}ms\n", m_unit_tests.size(), m_unit_tests_pass_count, m_unit_tests_fail_count, m_unit_tests_time_taken.count());

			if (pRunPerformanceTests)
			{
				output += std::format("----------------- PERFORMANCE TESTS -----------------\n");
				output += std::format("TOTAL TESTS: {}\nTIME TAKEN: {}ms\n", m_performance_tests.size(), m_performance_tests_time_taken.count());
			}
			output += seperator;
		}

		LOG("{}", output);
	}

} // namespace Test