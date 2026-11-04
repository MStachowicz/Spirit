#include "Label.hpp"

#include "Utility/Serialise.hpp"

namespace Component
{
	void Label::serialise(Utility::BinaryWriter& p_out, const Label& p_label)
	{
		p_out.write(p_label.m_name);
	}
	Label Label::deserialise(Utility::BinaryReader& p_in)
	{
		return Label{p_in.read<std::string>()};
	}
	static_assert(Utility::Is_Serializable_v<Label>, "Label is not serializable, check that the required functions are implemented.");
}