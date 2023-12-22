#include "TestManager.hpp"

namespace Test
{
	std::string to_string(const std::source_location& p_location)
	{
		return std::format("{}:{}", p_location.file_name(), p_location.line());
	}

	TestManager::TestManager(const std::string& p_name) noexcept
		: m_name{p_name}
		, m_unit_tests_pass_count{0}
		, m_unit_tests_fail_count{0}
		, m_unit_tests_failed_messages{""}
		, section_name_lengths{}
		, running_section_name{""}
	{}

	void TestManager::run_unit_test(const bool& p_condition, const std::string& p_name, const std::string& p_fail_message)
	{
		if (p_condition)
		{// Print passed and the running section name
			if (!running_section_name.empty())
				printf("PASSED %s - %s\n", running_section_name.c_str(), p_name.c_str());
			else
				printf("PASSED %s\n", p_name.c_str());

			m_unit_tests_pass_count++;
		}
		else
		{
			if (!running_section_name.empty())
			{
				m_unit_tests_failed_messages += running_section_name + " " + p_name + ":\n" + p_fail_message + "\n\n";
				printf("FAILED %s - %s\n", running_section_name.c_str(), p_name.c_str());
			}
			else
			{
				m_unit_tests_failed_messages += p_name + ":\n" + p_fail_message + "\n\n";
				printf("FAILED %s\n", p_name.c_str());
			}

			m_unit_tests_fail_count++;
		}
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

} // namespace Test