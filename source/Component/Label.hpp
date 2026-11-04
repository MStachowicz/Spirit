#pragma once

#include "Utility/Serialise.hpp"

#include <string>

namespace Component
{
	class Label
	{
	public:
		constexpr static size_t Persistent_ID = 12;

		Label(const std::string& p_name = "")
			: m_name{p_name}
		{}

		static void serialise(Utility::BinaryWriter& p_out, const Label& p_label);
		static Label deserialise(Utility::BinaryReader& p_in);

		std::string m_name;
	};
}