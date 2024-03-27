#pragma once

#include <string>

namespace Component
{
	class Label
	{
	public:
		constexpr static size_t Persistent_ID = 12;

		Label(const std::string& pName)
			: mName{pName}
		{}

		std::string mName;
	};
}