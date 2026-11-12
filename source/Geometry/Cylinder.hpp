#pragma once

namespace Geometry
{
	// A cylinder centred on the origin of its own space with its axis along +Y, from y = -m_half_height to +m_half_height.
	// Where it sits and how it is turned are passed separately, e.g. from a Transform, so a shape never holds a second copy of an
	// entity's pose that could disagree with it.
	class Cylinder
	{
	public:
		void draw_UI() const;

		float m_half_height;
		float m_radius;
	};
}