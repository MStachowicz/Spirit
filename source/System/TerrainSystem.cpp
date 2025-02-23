#include "TerrainSystem.hpp"
#include "SceneSystem.hpp"

#include "Component/FirstPersonCamera.hpp"
#include "Component/Terrain.hpp"
#include "Component/Transform.hpp"

#include "ECS/Storage.hpp"

namespace System
{
	void TerrainSystem::update(Scene& p_scene, float aspect_ratio)
	{
		std::optional<glm::vec3> player_pos;
		std::optional<float> view_distance;

		p_scene.m_entities.foreach([&](Component::FirstPersonCamera& p_camera, Component::Transform& p_transform)
		{
			if (p_camera.m_primary)
			{
				view_distance = p_camera.get_maximum_view_distance(aspect_ratio);
				player_pos    = p_transform.m_position;
				return;
			}
		});

		if (player_pos && view_distance)
		{
			p_scene.m_entities.foreach([&](Component::Terrain& p_terrain)
			{
				p_terrain.update(*player_pos, *view_distance);
			});
		}
	}
} // namespace System