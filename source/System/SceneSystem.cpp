#include "SceneSystem.hpp"
#include "AssetManager.hpp"

#include "Component/FirstPersonCamera.hpp"
#include "Component/Collider.hpp"
#include "Component/Input.hpp"
#include "Component/Label.hpp"
#include "Component/Lights.hpp"
#include "Component/Mesh.hpp"
#include "Component/ParticleEmitter.hpp"
#include "Component/RigidBody.hpp"
#include "Component/Terrain.hpp"
#include "Component/Texture.hpp"
#include "Component/Transform.hpp"

#include "Geometry/Geometry.hpp"

#include "Utility/Config.hpp"
#include "Utility/MeshBuilder.hpp"

namespace System
{
	SceneSystem::SceneSystem(System::AssetManager& p_asset_manager)
		: m_asset_manager{p_asset_manager}
		, m_scenes{}
		, m_current_scene_index{0}
	{
		auto& scene = add_scene();
		set_current_scene(scene);
		add_default_camera(scene);
		construct_2_sphere_scene(scene);

		Component::Terrain terrain({0.f, 5.f, -100.f}, 100, 100, 20);
		terrain.m_texture = m_asset_manager.get_texture(Config::Texture_Directory / "GrassTile.png");
		scene.m_entities.add_entity(terrain);
	}

	void SceneSystem::set_current_scene(const Scene& p_scene)
	{
		// To set the current scene, find the matching scene pointer in the m_scenes vector and set the m_current_scene_index to the index of the scene.
		auto it = std::find_if(m_scenes.begin(), m_scenes.end(), [&](const std::unique_ptr<System::Scene>& scene) { return scene.get() == &p_scene; });
		ASSERT_THROW(it != m_scenes.end(), "Scene not found in SceneSystem. Call add_scene() to add the scene to the SceneSystem before calling set_current_scene.");
		m_current_scene_index = std::distance(m_scenes.begin(), it);
	}

	void Scene::update(float aspect_ratio, Component::ViewInformation* view_info_override /*= nullptr*/)
	{
		{// Update scene bounds
			m_bound.m_min = glm::vec3(0.f);
			m_bound.m_max = glm::vec3(0.f);

			m_entities.foreach([&](ECS::Entity p_entity, Component::Transform& p_transform, Component::Mesh& p_mesh)
			{
				if (m_entities.has_components<Component::Collider>(p_entity))
				{
					auto& collider = m_entities.get_component<Component::Collider>(p_entity);
					m_bound.unite(collider.m_world_AABB);
				}
				else
				{
					const auto world_AABB = Geometry::AABB::transform(p_mesh.m_mesh->AABB, p_transform.m_position, glm::mat4_cast(p_transform.m_orientation), p_transform.m_scale);
					m_bound.unite(world_AABB);
				}
			});
		}
		{// Update the view information
			if (view_info_override)
			{
				m_view_information = *view_info_override;
			}
			else
			{
				m_entities.foreach([&](Component::FirstPersonCamera& p_camera, Component::Transform& p_transform)
				{
					if (p_camera.m_primary)
					{
						m_view_information.m_view_position = {p_transform.m_position, 1.f};
						m_view_information.m_view          = p_camera.view(p_transform.m_position);// glm::lookAt(p_transform.m_position, p_transform.m_position + p_transform.m_direction, camera_up);
						m_view_information.m_projection    = glm::perspective(glm::radians(p_camera.m_FOV), aspect_ratio, p_camera.m_near, p_camera.m_far);
						return;
					}
				});
			}
		}
	}

	void Scene::serialise(std::ostream& p_out, uint16_t p_version, const Scene& p_Scene)
	{
		ECS::Storage::serialise(p_out, p_version, p_Scene.m_entities);
	}

	Scene Scene::deserialise(std::istream& p_in, uint16_t p_version)
	{
		Scene scene;
		scene.m_entities = ECS::Storage::deserialise(p_in, p_version);
		// TODO: Update the scene bounds and view information after deserialising
		return scene;
	}

	void SceneSystem::add_default_camera(Scene& p_scene)
	{
		Component::Transform camera_transform;
		camera_transform.m_position = {0.f, 7.f, 12.5f};
		auto camera = Component::FirstPersonCamera(glm::vec3(0.f, -0.5f, 0.5f), true);
		camera.look_at(glm::vec3(0.f), camera_transform.m_position);

		p_scene.m_entities.add_entity(
			camera_transform,
			camera,
			Component::Label("Camera"),
			Component::RigidBody(false),
			Component::Input(Component::Input::Camera_Move_Look));
	}

	// Lines up all the available primitive meshes along the x axis with the camera facing them.
	void SceneSystem::primitives_scene(Scene& p_scene)
	{
		{ // Plane/quad
			auto transform    = Component::Transform{glm::vec3(0.f, 0.f, 0.f)};
			transform.m_scale  = glm::vec3(10.f, 1.f, 10.f);

			p_scene.m_entities.add_entity(
				Component::Label{"Floor"},
				Component::RigidBody{},
				Component::Texture{m_asset_manager.get_texture(Config::Texture_Directory / "wood_floor.png")},
				transform,
				Component::Mesh{m_asset_manager.m_quad},
				Component::Collider{});
		}

		constexpr float mesh_count   = 5.f;
		constexpr float mesh_width   = 2.f;
		constexpr float mesh_padding = 1.f;
		constexpr float start_x      = -((mesh_count - 1.f) / 2.f) * (mesh_width + mesh_padding);
		constexpr float start_y      = 2.f;
		constexpr float increment    = mesh_width + mesh_padding;

		float running_x = start_x;

		{ // Textured cube
			Component::Texture texture;
			texture.m_diffuse  = m_asset_manager.get_texture(Config::Texture_Directory / "metalContainerDiffuse.png");
			texture.m_specular = m_asset_manager.get_texture(Config::Texture_Directory / "metalContainerSpecular.png");

			p_scene.m_entities.add_entity(
				Component::Label{"Cube"},
				Component::RigidBody{},
				Component::Transform{glm::vec3(running_x, start_y, -mesh_width)},
				Component::Mesh{m_asset_manager.m_cube},
				Component::Collider{},
				texture);
			running_x += increment;
		}
		{ // Cone
			p_scene.m_entities.add_entity(
				Component::Label{"Cone"},
				Component::RigidBody{},
				Component::Transform{glm::vec3(running_x, start_y, -mesh_width)},
				Component::Mesh{m_asset_manager.m_cone},
				Component::Collider{});
			running_x += increment;
		}
		{ // Cylinder

			p_scene.m_entities.add_entity(
				Component::Label{"Cylinder"},
				Component::RigidBody{},
				Component::Transform{glm::vec3(running_x, start_y, -mesh_width)},
				Component::Mesh{m_asset_manager.m_cylinder},
				Component::Collider{});
			running_x += increment;
		}
		{ // quad
			p_scene.m_entities.add_entity(
				Component::Label{"Plane"},
				Component::RigidBody{},
				Component::Transform{glm::vec3(running_x, start_y, -mesh_width)},
				Component::Mesh{m_asset_manager.m_quad},
				Component::Collider{});
			running_x += increment;
		}
		{ // Sphere
			p_scene.m_entities.add_entity(
				Component::Label{"Sphere"},
				Component::RigidBody{},
				Component::Transform{glm::vec3(running_x, start_y, -mesh_width)},
				Component::Mesh{m_asset_manager.m_sphere},
				Component::Collider{});
			running_x += increment;
		}
		{ // Lights
			p_scene.m_entities.add_entity(Component::Label{"Directional light 1"}, Component::DirectionalLight{glm::vec3(0.f, -1.f, 0.f), 0.f, 0.5f});

			p_scene.m_entities.add_entity(Component::Label{"Point light 1"}, Component::PointLight{glm::vec3(6.f, 3.2f, -4.5f)});

			{ // Red point light in-front of the box.
				auto point_light      = Component::PointLight{};
				point_light.m_position = glm::vec3(-8.f, start_y, 1.f);
				point_light.m_colour   = glm::vec3(1.f, 0.f, 0.f);
				p_scene.m_entities.add_entity(Component::Label{"Point light 2"}, point_light);
			}
			{ // Spotlight over the box pointing down onto it.
				auto spotlight              = Component::SpotLight{};
				spotlight.m_position         = glm::vec3(start_x, 5.f, -mesh_width);
				spotlight.m_colour           = glm::vec3(0.f, 0.f, 1.f);
				spotlight.m_direction        = glm::vec3(0.f, -.1f, 0.f);
				spotlight.m_diffuse_intensity = 3.f;
				p_scene.m_entities.add_entity(Component::Label{"Spotlight 1"}, spotlight);
			}
		}

		{ // Particle
			auto particle_emitter = Component::ParticleEmitter{m_asset_manager.get_texture(Config::Texture_Directory / "smoke.png")};
			p_scene.m_entities.add_entity(Component::Label{"Particle emitter"}, particle_emitter);
		}
	}

	void SceneSystem::constructBoxScene(Scene& p_scene)
	{
		const auto containerDiffuse  = Config::Texture_Directory / "metalContainerDiffuse.png";
		const auto containerSpecular = Config::Texture_Directory / "metalContainerSpecular.png";

		{// Cubes
			for (size_t i = 0; i < 100; i += 2)
			{
				Component::Texture texture;
				texture.m_diffuse = m_asset_manager.get_texture(containerDiffuse);
				texture.m_specular = m_asset_manager.get_texture(containerSpecular);

				p_scene.m_entities.add_entity(
					Component::Label("Cube " + std::to_string((i / 2) + 1)),
					Component::Mesh(m_asset_manager.m_cube),
					Component::Transform{glm::vec3(i, 0.f, 0.f)},
					Component::Collider{},
					Component::RigidBody{},
					texture);
			}
		}
		{// Lights
			{// Point light
				const std::array<glm::vec3, 4> pointLightPositions = {
					glm::vec3(0.7f, 1.7f, 2.0f),
					glm::vec3(0.0f, 1.0f, -3.0f),
					glm::vec3(2.3f, 3.3f, -4.0f),
					glm::vec3(-4.0f, 2.0f, -12.0f)};
				const std::array<glm::vec3, 4> pointLightColours = {
					glm::vec3(0.f, 0.f, 1.f),
					glm::vec3(1.f),
					glm::vec3(1.f),
					glm::vec3(1.f)};

				for (size_t i = 0; i < pointLightPositions.size(); i++)
				{
					Component::PointLight pointLight;
					pointLight.m_position = pointLightPositions[i];
					pointLight.m_colour   = pointLightColours[i];
					p_scene.m_entities.add_entity(Component::Label("Point light " + std::to_string(i)), pointLight);
				}
			}
			{// Directional light
				Component::DirectionalLight directionalLight;
				directionalLight.m_direction = glm::vec3(-0.2f, -1.0f, -0.3f);
				directionalLight.m_ambient_intensity = 0.7f;
				directionalLight.m_diffuse_intensity = 0.3f;
				p_scene.m_entities.add_entity(Component::Label("Directional light"), directionalLight);
			}
			{// Spotlight
				Component::Label name = Component::Label("Spot light");
				p_scene.m_entities.add_entity(Component::SpotLight(), name);
			}
		}
	}
	void SceneSystem::construct_2_sphere_scene(Scene& p_scene)
	{
		auto mb = Utility::MeshBuilder<Data::Vertex, OpenGL::PrimitiveMode::Triangles, true>{};
		mb.add_icosphere(glm::vec3(0.f), 1.f, 1);
		auto icosphere_mesh = mb.get_mesh();
		auto icosphere_meshref = m_asset_manager.insert(std::move(icosphere_mesh));

		p_scene.m_entities.add_entity(
			Component::Label{"Directional light 1"},
			Component::DirectionalLight{glm::vec3(0.f, -1.f, 0.f), 0.3f, 0.5f});

		p_scene.m_entities.add_entity(
			Component::Label{"Sphere 1"},
			Component::RigidBody{},
			Component::Transform{glm::vec3(50.f, 30.f, -50.f)},
			Component::Mesh{icosphere_meshref},
			Component::Texture{glm::vec4(0.5f, 0.5f, 0.5f, 0.6f)}, // Grey
			Component::Collider{});

		p_scene.m_entities.add_entity(
			Component::Label{"Sphere 2"},
			Component::RigidBody{},
			Component::Transform{glm::vec3(55.f, 30.f, -50.f)},
			Component::Mesh{icosphere_meshref},
			Component::Texture{glm::vec4(1.f, 0.647f, 0.f, 0.6f)}, // Orange
			Component::Collider{});
	}
	void SceneSystem::constructBouncingBallScene(Scene& p_scene)
	{
		const auto containerDiffuse  = Config::Texture_Directory / "metalContainerDiffuse.png";
		const auto containerSpecular = Config::Texture_Directory / "metalContainerSpecular.png";

		{ // Ball
			Component::Transform transform;
			transform.m_position = glm::vec3(-10.f, 5.f, 0.f);

			Component::Label name = Component::Label("Sphere");

			Component::Mesh mesh = Component::Mesh(m_asset_manager.m_sphere);
			Component::Texture texture;
			texture.m_diffuse = m_asset_manager.get_texture(containerDiffuse);
			texture.m_specular = m_asset_manager.get_texture(containerSpecular);

			Component::RigidBody rigidBody;
			rigidBody.m_mass = 1.f;
			p_scene.m_entities.add_entity(mesh, transform, Component::Collider(), rigidBody, name);
		}
		{ // Floor
			auto transform     = Component::Transform{glm::vec3(0.f, 0.f, 0.f)};
			transform.m_scale  = glm::vec3(10.f, 1.f, 10.f);

			p_scene.m_entities.add_entity(
				Component::Label{"Floor"},
				Component::RigidBody{},
				Component::Texture{m_asset_manager.get_texture(Config::Texture_Directory / "wood_floor.png")},
				transform,
				Component::Mesh{m_asset_manager.m_quad},
				Component::Collider{});
		}
		{// Lights
			{// Point light
				const std::array<glm::vec3, 4> pointLightPositions = {
					glm::vec3(0.7f, 1.7f, 2.0f),
					glm::vec3(0.0f, 1.0f, -3.0f),
					glm::vec3(2.3f, 3.3f, -4.0f),
					glm::vec3(-4.0f, 2.0f, -12.0f)};
				const std::array<glm::vec3, 4> pointLightColours = {
					glm::vec3(0.f, 0.f, 1.f),
					glm::vec3(1.f),
					glm::vec3(1.f),
					glm::vec3(1.f)};

				for (size_t i = 0; i < pointLightPositions.size(); i++)
				{
					Component::PointLight pointLight;
					pointLight.m_position = pointLightPositions[i];
					pointLight.m_colour   = pointLightColours[i];
					p_scene.m_entities.add_entity(Component::Label("Point light " + std::to_string(i)), pointLight);
				}
			}
			{// Directional light
				Component::DirectionalLight directionalLight;
				directionalLight.m_direction = glm::vec3(-0.2f, -1.0f, -0.3f);
				p_scene.m_entities.add_entity(Component::Label("Directional light"), directionalLight);
			}
			{// Spotlight
				p_scene.m_entities.add_entity(Component::Label("Spot light"), Component::SpotLight());
			}
		}
	}
}