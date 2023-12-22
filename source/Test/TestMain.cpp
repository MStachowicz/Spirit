#include "Test/TestManager.hpp"
#include "Test/Tests/ECSTester.hpp"
#include "Test/Tests/GeometryTester.hpp"
#include "Test/Tests/ResourceManagerTester.hpp"

#include <cstring>
#include "Utility/Stopwatch.hpp"

int main(int argc, char* argv[])
{
	bool should_run_perf_tests = false;
	if (argc == 2 && strcmp(argv[1], "--performance") == 0)
		should_run_perf_tests = true;
	else if (argc == 1)
		should_run_perf_tests = false;
	else
	{
		printf("Usage: %s [flags]\n", argv[0]);
		printf("\nFlags:\n");
		printf("  --performance           Additionally run performance tests\n");
		printf("\n");
		exit(1);
	}

	const char* seperator = "--------------------------------------------------\n";

	std::vector<std::unique_ptr<Test::TestManager>> test_managers;
	test_managers.emplace_back(std::make_unique<Test::ECSTester>());
	test_managers.emplace_back(std::make_unique<Test::GeometryTester>());
	test_managers.emplace_back(std::make_unique<Test::ResourceManagerTester>());

	size_t unit_test_overall_pass_count    = 0;
	size_t unit_test_overall_fail_count    = 0;
	std::string unit_tests_failed_messages = "";

	Utility::Stopwatch all_unit_tests_stopwatch;
	for (auto& tester : test_managers)
	{
		printf("***************** STARTING %s UNIT TESTS *****************\n", tester->m_name.c_str());
		Utility::Stopwatch tester_stopwatch;
		tester->run_unit_tests();
		printf("***************** %s SUMMARY *****************\nTOTAL TESTS: %zu\nPASSED: %zu\nFAILED: %zu\nTIME TAKEN: %fms\n%s\n\n",
		       tester->m_name.c_str(),
		       tester->m_unit_tests_pass_count + tester->m_unit_tests_fail_count,
		       tester->m_unit_tests_pass_count,
		       tester->m_unit_tests_fail_count,
		       tester_stopwatch.duration_since_start<float, std::milli>().count(),
		       seperator);

		unit_test_overall_pass_count += tester->m_unit_tests_pass_count;
		unit_test_overall_fail_count += tester->m_unit_tests_fail_count;
		unit_tests_failed_messages += tester->m_unit_tests_failed_messages;
	}
	printf("\n\n***************** OVERALL SUMMARY *****************\nTOTAL TESTS: %zu\nPASSED: %zu\nFAILED: %zu\nTIME TAKEN: %fms\n%s",
	       unit_test_overall_pass_count + unit_test_overall_fail_count,
	       unit_test_overall_pass_count,
	       unit_test_overall_fail_count,
	       all_unit_tests_stopwatch.duration_since_start<float, std::milli>().count(),
	       seperator);

	if (unit_test_overall_fail_count > 0)
		printf("***************** FAILED TESTS *****************\n%s", unit_tests_failed_messages.c_str());

	return unit_test_overall_fail_count;
}