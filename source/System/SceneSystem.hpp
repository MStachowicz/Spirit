#pragma once

#include "ECS/Storage.hpp"
#include "Geometry/AABB.hpp"
#include "Component/ViewInformation.hpp"

#include <memory>

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

		static void serialise(std::ostream& p_out, uint16_t p_version, const Scene& p_Scene);
		static Scene deserialise(std::istream& p_in, uint16_t p_version);
	};

	class SceneSystem
	{
		TextureSystem& m_texture_system;
		MeshSystem& m_mesh_system;

		std::vector<std::unique_ptr<Scene>> m_scenes;
		size_t m_current_scene_index;

	public:
		SceneSystem(TextureSystem& p_texture_system, MeshSystem& p_mesh_system);

		Scene& get_current_scene()                                      { return *m_scenes[m_current_scene_index]; }
		const Scene& get_current_scene() const                          { return *m_scenes[m_current_scene_index]; }
		void set_current_scene(const Scene& p_scene);

		Scene& add_scene()                                              { return *m_scenes.emplace_back(std::make_unique<Scene>()); }
		ECS::Storage& get_current_scene_entities()                      { return m_scenes[m_current_scene_index]->m_entities; }
		const Component::ViewInformation& get_current_scene_view_info() { return m_scenes[m_current_scene_index]->m_view_information; }

	private:
		void add_default_camera(Scene& p_scene);
		void constructBouncingBallScene(Scene& p_scene);
		void constructBoxScene(Scene& p_scene);
		void construct_2_sphere_scene(Scene& p_scene);
		void primitives_scene(Scene& p_scene);
	};
} // namespace System