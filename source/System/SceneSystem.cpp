#include "SceneSystem.hpp"
#include "Component/Camera.hpp"
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
#include "System/MeshSystem.hpp"
#include "System/TextureSystem.hpp"

#include "Geometry/Geometry.hpp"

#include "Config.hpp"

namespace System
{
	SceneSystem::SceneSystem(System::TextureSystem& pTextureSystem, System::MeshSystem& pMeshSystem)
		: mTextureSystem(pTextureSystem)
		, mMeshSystem(pMeshSystem)
		, m_scene{}
	{
		add_default_camera();
		primitives_scene();
		//constructBoxScene();
		//constructBouncingBallScene();
	}

	Component::Camera* Scene::get_primary_camera()
	{
		Component::Camera* primaryCamera = nullptr;

		m_entities.foreach([&primaryCamera](Component::Camera& pCamera)
		{
			if (pCamera.m_primary)
			{
				primaryCamera = &pCamera;
				return;
			}
		});
		return primaryCamera;
	}

	void SceneSystem::update_scene_bounds()
	{
		m_scene.m_bound.mMin = glm::vec3(0.f);
		m_scene.m_bound.mMax = glm::vec3(0.f);

		getCurrentScene().foreach([&scene = m_scene.m_entities, &scene_bounds = m_scene.m_bound](ECS::Entity p_entity, Component::Transform& p_transform, Component::Mesh& p_mesh)
		{
			if (scene.hasComponents<Component::Collider>(p_entity))
			{
				auto& collider = scene.getComponent<Component::Collider>(p_entity);
				scene_bounds.unite(collider.m_world_AABB);
			}
			else
			{
				const auto world_AABB = Geometry::AABB::transform(p_mesh.m_mesh->AABB, p_transform.mPosition, glm::mat4_cast(p_transform.mOrientation), p_transform.mScale);
				scene_bounds.unite(world_AABB);
			}
		});
	}

	void SceneSystem::add_default_camera()
	{
		Component::Transform camera_transform;
		camera_transform.mPosition = {0.f, 7.f, 12.5f};
		auto camera = Component::Camera(glm::vec3(0.f, -0.5f, 0.5f), true);
		camera.look_at(glm::vec3(0.f), camera_transform.mPosition);

		m_scene.m_entities.addEntity(
			camera_transform,
			camera,
			Component::Label("Camera"),
			Component::RigidBody(),
			Component::Input(Component::Input::Camera_Move_Look));
	}

	// Lines up all the available primitive meshes along the x axis with the camera facing them.
	void SceneSystem::primitives_scene()
	{
		{ // Plane/quad
			auto transform    = Component::Transform{glm::vec3(0.f, 0.f, 0.f)};
			transform.mScale  = glm::vec3(10.f, 1.f, 10.f);

			m_scene.m_entities.addEntity(
				Component::Label{"Floor"},
				Component::RigidBody{},
				Component::Texture{mTextureSystem.getTexture(Config::Texture_Directory / "wood_floor.png")},
				transform,
				Component::Mesh{mMeshSystem.m_quad},
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
			texture.mDiffuse  = mTextureSystem.getTexture(Config::Texture_Directory / "metalContainerDiffuse.png");
			texture.mSpecular = mTextureSystem.getTexture(Config::Texture_Directory / "metalContainerSpecular.png");

			m_scene.m_entities.addEntity(
				Component::Label{"Cube"},
				Component::RigidBody{},
				Component::Transform{glm::vec3(running_x, start_y, -mesh_width)},
				Component::Mesh{mMeshSystem.m_cube},
				Component::Collider{},
				texture);
			running_x += increment;
		}
		{ // Cone
			m_scene.m_entities.addEntity(
				Component::Label{"Cone"},
				Component::RigidBody{},
				Component::Transform{glm::vec3(running_x, start_y, -mesh_width)},
				Component::Mesh{mMeshSystem.m_cone},
				Component::Collider{});
			running_x += increment;
		}
		{ // Cylinder

			m_scene.m_entities.addEntity(
				Component::Label{"Cylinder"},
				Component::RigidBody{},
				Component::Transform{glm::vec3(running_x, start_y, -mesh_width)},
				Component::Mesh{mMeshSystem.m_cylinder},
				Component::Collider{});
			running_x += increment;
		}
		{ // quad
			m_scene.m_entities.addEntity(
				Component::Label{"Plane"},
				Component::RigidBody{},
				Component::Transform{glm::vec3(running_x, start_y, -mesh_width)},
				Component::Mesh{mMeshSystem.m_quad},
				Component::Collider{});
			running_x += increment;
		}
		{ // Sphere
			m_scene.m_entities.addEntity(
				Component::Label{"Sphere"},
				Component::RigidBody{},
				Component::Transform{glm::vec3(running_x, start_y, -mesh_width)},
				Component::Mesh{mMeshSystem.m_sphere},
				Component::Collider{});
			running_x += increment;
		}
		{ // Lights
			m_scene.m_entities.addEntity(Component::Label{"Directional light 1"}, Component::DirectionalLight{glm::vec3(0.f, -1.f, 0.f), 0.f, 0.5f});

			m_scene.m_entities.addEntity(Component::Label{"Point light 1"}, Component::PointLight{glm::vec3(6.f, 3.2f, -4.5f)});

			{ // Red point light in-front of the box.
				auto point_light      = Component::PointLight{};
				point_light.mPosition = glm::vec3(-8.f, start_y, 1.f);
				point_light.mColour   = glm::vec3(1.f, 0.f, 0.f);
				m_scene.m_entities.addEntity(Component::Label{"Point light 2"}, point_light);
			}
			{ // Spotlight over the box pointing down onto it.
				auto spotlight              = Component::SpotLight{};
				spotlight.mPosition         = glm::vec3(start_x, 5.f, -mesh_width);
				spotlight.mColour           = glm::vec3(0.f, 0.f, 1.f);
				spotlight.mDirection        = glm::vec3(0.f, -.1f, 0.f);
				spotlight.mDiffuseIntensity = 3.f;
				m_scene.m_entities.addEntity(Component::Label{"Spotlight 1"}, spotlight);
			}
		}

		{ // Particle
			auto particle_emitter = Component::ParticleEmitter{mTextureSystem.getTexture(Config::Texture_Directory / "smoke.png")};
			particle_emitter.sort_by_distance_to_camera = true;
			m_scene.m_entities.addEntity(Component::Label{"Particle emitter"}, particle_emitter);
		}
		{ // Terrain
			auto terrain = Component::Terrain{100, 100};
			m_scene.m_entities.addEntity(Component::Label{"Terrain"}, terrain);
		}
	}

	void SceneSystem::constructBoxScene()
	{
		const auto containerDiffuse  = Config::Texture_Directory / "metalContainerDiffuse.png";
		const auto containerSpecular = Config::Texture_Directory / "metalContainerSpecular.png";

		{// Cubes
			for (size_t i = 0; i < 100; i += 2)
			{
				Component::Texture texture;
				texture.mDiffuse = mTextureSystem.getTexture(containerDiffuse);
				texture.mSpecular = mTextureSystem.getTexture(containerSpecular);

				m_scene.m_entities.addEntity(
					Component::Label("Cube " + std::to_string((i / 2) + 1)),
					Component::Mesh(mMeshSystem.m_cube),
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
					pointLight.mPosition = pointLightPositions[i];
					pointLight.mColour   = pointLightColours[i];
					m_scene.m_entities.addEntity(Component::Label("Point light " + std::to_string(i)), pointLight);
				}
			}
			{// Directional light
				Component::DirectionalLight directionalLight;
				directionalLight.mDirection = glm::vec3(-0.2f, -1.0f, -0.3f);
				directionalLight.mAmbientIntensity = 0.7f;
				directionalLight.mDiffuseIntensity = 0.3f;
				m_scene.m_entities.addEntity(Component::Label("Directional light"), directionalLight);
			}
			{// Spotlight
				Component::Label name = Component::Label("Spot light");
				m_scene.m_entities.addEntity(Component::SpotLight(), name);
			}
		}
	}
	void SceneSystem::constructBouncingBallScene()
	{
		const auto containerDiffuse  = Config::Texture_Directory / "metalContainerDiffuse.png";
		const auto containerSpecular = Config::Texture_Directory / "metalContainerSpecular.png";

		{ // Ball
			Component::Transform transform;
			transform.mPosition = glm::vec3(-10.f, 5.f, 0.f);

			Component::Label name = Component::Label("Sphere");

			Component::Mesh mesh = Component::Mesh(mMeshSystem.m_sphere);
			Component::Texture texture;
			texture.mDiffuse = mTextureSystem.getTexture(containerDiffuse);
			texture.mSpecular = mTextureSystem.getTexture(containerSpecular);

			Component::RigidBody rigidBody;
			rigidBody.mMass = 1.f;
			m_scene.m_entities.addEntity(mesh, transform, Component::Collider(), rigidBody, name);
		}
		{ // Floor
			Component::Transform transform;
			transform.rotateEulerDegrees(glm::vec3(-90.f, 0.f, 0.f));
			transform.mScale = glm::vec3(50.f);
			Component::RigidBody rigidBody;
			rigidBody.mMass = 1.f;
			m_scene.m_entities.addEntity(
				Component::Label("Floor"),
				Component::Mesh{mMeshSystem.m_plane},
				transform,
				Component::Collider(),
				rigidBody);
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

				for (auto i = 0; i < pointLightPositions.size(); i++)
				{
					Component::PointLight pointLight;
					pointLight.mPosition = pointLightPositions[i];
					pointLight.mColour   = pointLightColours[i];
					m_scene.m_entities.addEntity(Component::Label("Point light " + std::to_string(i)), pointLight);
				}
			}
			{// Directional light
				Component::DirectionalLight directionalLight;
				directionalLight.mDirection = glm::vec3(-0.2f, -1.0f, -0.3f);
				m_scene.m_entities.addEntity(Component::Label("Directional light"), directionalLight);
			}
			{// Spotlight
				m_scene.m_entities.addEntity(Component::Label("Spot light"), Component::SpotLight());
			}
		}
	}
}