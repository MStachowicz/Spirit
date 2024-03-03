#pragma once

#include "ECS/Storage.hpp"
#include "Geometry/AABB.hpp"
#include "Component/ViewInformation.hpp"

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
		Geometry::AABB m_bound; // The bounding box of the m_entities in the scene. Used by rendering.
		Component::ViewInformation m_view_information; // Rendering depends on the ViewInformation of the active camera.

		// When the state of the scene changes update the m_bound and m_view_information.
		// Should be called when the scene is first created, when entities are added/removed/changed, when the aspect ratio changes or when the editor changes the scene.
		void update(float aspect_ratio, Component::ViewInformation* view_info_override = nullptr);
	};

	class SceneSystem
	{
		TextureSystem& m_texture_system;
		MeshSystem& m_mesh_system;
		Scene m_scene;

	public:
		SceneSystem(TextureSystem& p_texture_system, MeshSystem& p_mesh_system);

		Scene& get_current_scene()                                      { return m_scene; }
		ECS::Storage& get_current_scene_entities()                      { return m_scene.m_entities; }
		const Component::ViewInformation& get_current_scene_view_info() { return m_scene.m_view_information; }

	private:
		void add_default_camera();
		void constructBouncingBallScene();
		void constructBoxScene();
		void construct_2_sphere_scene();
		void primitives_scene();
	};
} // namespace System