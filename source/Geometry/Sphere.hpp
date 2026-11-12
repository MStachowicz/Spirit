#pragma once

namespace Geometry
{
	// A sphere centred on the origin of its own space. Where it sits is passed separately, e.g. from a Transform, so a shape never
	// holds a second copy of an entity's position that could disagree with it.
	class Sphere
	{
	public:
		void draw_UI() const;

		float m_radius;
	};
}