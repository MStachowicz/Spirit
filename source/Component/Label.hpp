#pragma once

#include <string>
#include <iostream>
#include <cstdint>

namespace Component
{
	class Label
	{
	public:
		constexpr static size_t Persistent_ID = 12;

		Label(const std::string& p_name = "")
			: m_name{p_name}
		{}

		static void serialise(std::ostream& p_out, uint16_t p_version, const Label& p_label);
		static Label deserialise(std::istream& p_in, uint16_t p_version);

		std::string m_name;
	};
}