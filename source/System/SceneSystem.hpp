#pragma once

#include "ECS/Storage.hpp"
#include "Geometry/AABB.hpp"

namespace Component
{
	class Camera;
}
namespace System
{
	class TextureSystem;
	class MeshSystem;

	class Scene
	{
	public:
		ECS::Storage m_entities;
		Geometry::AABB m_bound;

		Component::Camera* get_primary_camera();
	};

	class SceneSystem
	{
		TextureSystem& m_texture_system;
		MeshSystem& m_mesh_system;

	public:
		Scene m_scene;

		SceneSystem(TextureSystem& p_texture_system, MeshSystem& p_mesh_system);
		ECS::Storage& get_current_scene() { return m_scene.m_entities; }
		void update_scene_bounds();

	private:
		void add_default_camera();
		void constructBouncingBallScene();
		void constructBoxScene();
		void construct_2_sphere_scene();
		void primitives_scene();
	};
} // namespace System