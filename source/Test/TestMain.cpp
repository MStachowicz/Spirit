#include "Test/TestManager.hpp"
#include "Utility/Logger.hpp"

#include <cstring>

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

	auto [pass_count, fail_count] = Test::run_unit_tests(should_run_perf_tests);

	std::string output;
	{
		output += std::format("***************** {} TEST SUMMARY *****************\n", "OVERALL");
		output += std::format("----------------- UNIT TESTS -----------------\n");
		output += std::format("TOTAL TESTS: {}\nPASSED: {}\nFAILED: {}\n", pass_count + fail_count, pass_count, fail_count);
		output += Test::seperator;
	}

	LOG("{}", output);

	return fail_count;
}