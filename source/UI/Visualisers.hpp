#pragma once

namespace ECS
{
	class Entity;
}
namespace System
{
	class Scene;
}
namespace UI
{
	void draw_frustrum_debugger(float aspect_ratio);
	void draw_tri_tri_debugger();
	void draw_GJK_debugger(ECS::Entity& p_entiy_1, ECS::Entity& p_entity_2, System::Scene& p_scene, int p_debug_step);
}