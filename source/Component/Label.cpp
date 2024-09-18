#include "Label.hpp"

#include "Utility/Serialise.hpp"

namespace Component
{
	void Label::serialise(std::ostream& p_out, uint16_t p_version, const Label& p_label)
	{
		Utility::write_binary(p_out, p_version, p_label.m_name);
	}
	Label Label::deserialise(std::istream& p_in, uint16_t p_version)
	{
		Label label("");
		Utility::read_binary(p_in, p_version, label.m_name);
		return label;
	}
	static_assert(Utility::Is_Serializable_v<Label>, "Label is not serializable, check that the required functions are implemented.");
}