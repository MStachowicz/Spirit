#pragma once

namespace Geometry
{
	// A right circular cone centred on the origin of its own space with its axis along +Y: the circular base at
	// y = -m_half_height tapers to the apex at y = +m_half_height. Where it sits and how it is turned are passed separately,
	// e.g. from a Transform, so a shape never holds a second copy of an entity's pose that could disagree with it.
	class Cone
	{
	public:
		void draw_UI() const;

		float m_half_height;
		float m_base_radius;
	};
}