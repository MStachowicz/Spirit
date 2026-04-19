#include "PhysicsSystemJolt.hpp"
#include "SceneSystem.hpp"

#include "Component/Collider.hpp"
#include "Component/Transform.hpp"

#include "Geometry/Shape.hpp"

#include "Utility/Performance.hpp"
#include "Utility/Utility.hpp"

#include "Jolt/Core/Factory.h"
#include "Jolt/Physics/Body/BodyActivationListener.h"
#include "Jolt/Physics/Body/BodyCreationSettings.h"
#include "Jolt/Physics/Collision/CastResult.h"
#include "Jolt/Physics/Collision/RayCast.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/Physics/Collision/Shape/SphereShape.h"
#include "Jolt/Physics/Collision/Shape/PlaneShape.h"
#include "Jolt/Physics/Collision/Shape/TaperedCylinderShape.h"
#include "Jolt/Physics/Collision/Shape/CylinderShape.h"
#include "Jolt/Physics/Collision/Shape/HeightFieldShape.h"
#include "Jolt/Physics/PhysicsSettings.h"
#include "Jolt/RegisterTypes.h"

#include "ECS/Storage.hpp"

#include <thread>

namespace System
{
	inline auto to_glm(const JPH::RVec3& v) {
#ifdef JPH_DOUBLE_PRECISION
		return glm::dvec3(v.GetX(), v.GetY(), v.GetZ());
#else
		return glm::vec3(v.GetX(), v.GetY(), v.GetZ());
#endif
	}
	inline auto to_glm(const JPH::Quat& q) {
#ifdef JPH_DOUBLE_PRECISION
		return glm::dquat(q.GetW(), q.GetX(), q.GetY(), q.GetZ());
#else
		return glm::quat(q.GetW(), q.GetX(), q.GetY(), q.GetZ());
#endif
	}
	inline auto to_geom(const JPH::AABox& box) { return Geometry::AABB{ to_glm(box.mMin), to_glm(box.mMax) }; }
	inline auto to_JPH(const glm::vec3& v) { return JPH::Vec3(v.x, v.y, v.z); }
	inline auto to_JPH(const glm::quat& q) { return JPH::Quat(q.x, q.y, q.z, q.w); }
	inline auto to_JPH(const Geometry::Ray& ray) { return JPH::RayCast{ to_JPH(ray.m_start), to_JPH(ray.m_direction) }; }
	inline auto to_JPH(BodySettings::MotionType motion_type) {
		switch (motion_type) {
			case BodySettings::MotionType::Static:    return JPH::EMotionType::Static;
			case BodySettings::MotionType::Kinematic: return JPH::EMotionType::Kinematic;
			case BodySettings::MotionType::Dynamic:   return JPH::EMotionType::Dynamic;
			default: ASSERT_FAIL("Unknown MotionType in to_JPH");
		}
	}

	void JoltTrace(const char* inFMT, ...)
	{
		// Format the message
		va_list list;
		va_start(list, inFMT);
		char buffer[1024];
		vsnprintf(buffer, sizeof(buffer), inFMT, list);
		va_end(list);
		LOG("[JOLT] {}", buffer);
	}
#ifdef Z_DEBUG
	bool JoltAssertFailed([[maybe_unused]] const char* inExpression, [[maybe_unused]] const char* inMessage, [[maybe_unused]] const char* inFile, [[maybe_unused]] JPH::uint inLine)
	{
		LOG_ERROR_NO_LOCATION("[JOLT] Assertion failed in file {} at line {}: ({}) {}", inFile, static_cast<unsigned int>(inLine), inExpression, (inMessage != nullptr ? inMessage : ""));
		return false;
	}
#endif // Z_DEBUG

	PhysicsSystemJolt::JoltInitWrapper::JoltInitWrapper()
	{
		JPH::RegisterDefaultAllocator();

		JPH::Trace = JoltTrace;
#ifdef Z_DEBUG
		JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = JoltAssertFailed);
#endif // Z_DEBUG

		JPH::Factory::sInstance = new JPH::Factory();
		// If you have your own custom shape types you probably need to register their handlers with the CollisionDispatch before calling this function.
		// If you implement your own default material (PhysicsMaterial::sDefault) make sure to initialize it before this function or else this function will create one for you.
		JPH::RegisterTypes();
	}

	PhysicsSystemJolt::PhysicsSystemJolt(SceneSystem& scene_system)
		: jolt_init{}
		, m_scene_system{scene_system}
		, max_physics_jobs{2048}
		, max_physics_barriers{8}
		, max_bodies{65536}
		, num_body_mutexes{0}
		, max_body_pairs{65536}
		, max_contact_constraints{10240}
		, temp_allocator_size{10 * 1024 * 1024} // 10 MB
		, temp_allocator{temp_allocator_size}
		, job_system{max_physics_jobs, max_physics_barriers, ((int)std::thread::hardware_concurrency()) - 1}
		, broad_phase_layer_interface{}
		, object_vs_broadphase_layer_filter{}
		, object_vs_object_layer_filter{}
		, physics_system{}
	{
		physics_system.Init(max_bodies, num_body_mutexes, max_body_pairs, max_contact_constraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);

		// auto ps = physics_system.GetPhysicsSettings();
		// ps.mPenetrationSlop = 0.0f;          // default is ~0.02
		// ps.mBaumgarte = 0.4f;                // stronger position correction (optional)
		// ps.mNumVelocitySteps = 10;           // more solver iterations (optional)
		// ps.mNumPositionSteps = 2;            // optional
		// physics_system.SetPhysicsSettings(ps);
	}

	PhysicsSystemJolt::~PhysicsSystemJolt()
	{
		m_scene_system.get_current_scene_entities().foreach([&](Component::Collider& collider)
		{
			if (collider.m_physics_system_handle->m_system == this)
				collider.m_physics_system_handle.reset();
		});

		// Unregisters all types with the factory and cleans up the default material
		JPH::UnregisterTypes();

		// Destroy the factory
		delete JPH::Factory::sInstance;
		JPH::Factory::sInstance = nullptr;
	}

	void PhysicsSystemJolt::register_pending_bodies()
	{
		PERF(PhysicsSystemJoltRegisterBodies);

		bool bodies_created = false;
		m_scene_system.get_current_scene_entities().foreach([&](ECS::Entity& p_entity, Component::Collider& collider, Component::Transform& transform)
		{
			if (!collider.m_physics_system_handle && collider.m_body_settings_cache)
			{
				collider.m_physics_system_handle = create_body(collider.m_body_settings_cache.value(), p_entity);
				// Keep m_body_settings_cache as persistent blueprint data so scene copies can recreate bodies.

				// Ensure the Jolt body position matches the ECS Transform, not just the shape's embedded center.
				set_position(collider.m_physics_system_handle.value(), transform.m_position);
				set_rotation(collider.m_physics_system_handle.value(), transform.m_orientation);
				bodies_created = true;
			}
		});

		// Only optimize broad phase after batch insertions, not every frame.
		if (bodies_created)
			physics_system.OptimizeBroadPhase();
	}

	void PhysicsSystemJolt::step(const DeltaTime& p_delta_time)
	{
		static_assert(std::is_same_v<DeltaTime, std::chrono::duration<float, std::ratio<1>>>, "PhysicsSystemJolt::step expects DeltaTime to be a duration in seconds with float precision.");
		PERF(PhysicsSystemJoltStep);

		if (m_pause_simulation)
			return;
		m_update_count++;

		const int cCollisionSteps = 1;
		physics_system.Update(p_delta_time.count(), cCollisionSteps, &temp_allocator, &job_system);

		// Update all transforms in the scene system to match the physics simulation.
		m_scene_system.get_current_scene_entities().foreach([&](Component::Collider& collider, Component::Transform& transform)
		{
			if (collider.m_physics_system_handle)
			{
				transform.m_position    = get_position(collider.m_physics_system_handle.value());
				transform.m_orientation = get_rotation(collider.m_physics_system_handle.value());
			}
		});
	}

	PhysicsSystemHandle PhysicsSystemJolt::create_body(const BodySettings& p_body_settings, const ECS::Entity& p_entity)
	{
		std::unique_ptr<JPH::ShapeSettings> body_shape_settings = nullptr;
		JPH::Vec3 pos(0.0f, 0.0f, 0.0f);
		JPH::Quat shape_rotation = JPH::Quat::sIdentity();
		std::visit([&body_shape_settings, &pos, &shape_rotation](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;

			if constexpr (std::is_same_v<T, Geometry::Sphere>)
			{
				pos = to_JPH(arg.m_center);
				body_shape_settings = std::make_unique<JPH::SphereShapeSettings>(arg.m_radius);
			}
			else if constexpr (std::is_same_v<T, Geometry::Plane>)
			{
				body_shape_settings = std::make_unique<JPH::PlaneShapeSettings>(JPH::Plane{to_JPH(arg.m_normal), arg.m_distance});
			}
			else if constexpr (std::is_same_v<T, Geometry::Cuboid>)
			{
				pos = to_JPH(arg.m_center);
				body_shape_settings = std::make_unique<JPH::BoxShapeSettings>(to_JPH(arg.m_half_extents));
			}
			else if constexpr (std::is_same_v<T, Geometry::Cone>)
			{
				pos = to_JPH(arg.center());
				body_shape_settings = std::make_unique<JPH::TaperedCylinderShapeSettings>(arg.height() / 2.f, 0.f, arg.m_base_radius);
				auto dir = glm::normalize(arg.m_top - arg.m_base);
				shape_rotation = to_JPH(Utility::get_rotation(glm::vec3(0.f, 1.f, 0.f), dir));
			}
			else if constexpr (std::is_same_v<T, Geometry::Cylinder>)
			{
				pos = to_JPH(arg.center());
				body_shape_settings = std::make_unique<JPH::CylinderShapeSettings>(arg.height() / 2.f, arg.m_radius);
				auto dir = glm::normalize(arg.m_top - arg.m_base);
				shape_rotation = to_JPH(Utility::get_rotation(glm::vec3(0.f, 1.f, 0.f), dir));
			}
			else if constexpr (std::is_same_v<T, Geometry::Quad>)
			{
				pos = to_JPH(arg.center());
				// Use a thin box instead of PlaneShape — PlaneShape is an infinite half-space whose AABB
				// extends half_extent deep behind the surface, making it unsuitable for finite quads.
				constexpr float thin_half_thickness = 0.01f;
				float he = arg.half_extent();
				body_shape_settings = std::make_unique<JPH::BoxShapeSettings>(JPH::Vec3(he, thin_half_thickness, he));
				auto dir = arg.normal();
				shape_rotation = to_JPH(Utility::get_rotation(glm::vec3(0.f, 1.f, 0.f), dir));
			}
			else static_assert(false, "non-exhaustive visitor!");
		}, p_body_settings.m_shape);

		auto result = body_shape_settings->Create();
		ASSERT_THROW(result.IsValid(), "Failed to create Jolt shape for entity ID {}: {}", p_entity.ID, result.GetError());

		auto body_create_settings = JPH::BodyCreationSettings(
			body_shape_settings.release(), // Transfer ownership to BodyCreationSettings, Jolt will take care of deleting it.
			pos,
			to_JPH(p_body_settings.m_rotation) * shape_rotation,
			to_JPH(p_body_settings.m_motion_type),
			p_body_settings.m_motion_type == BodySettings::MotionType::Static ? Layers::NON_MOVING : Layers::MOVING);
		body_create_settings.mUserData = static_cast<JPH::uint64>(p_entity.ID);

		auto& body_interface = get_body_interface_no_lock();
		auto activation = (p_body_settings.m_motion_type == BodySettings::MotionType::Static)
			? JPH::EActivation::DontActivate
			: JPH::EActivation::Activate;
		auto body_ID = body_interface.CreateAndAddBody(body_create_settings, activation);
		ASSERT_THROW(!body_ID.IsInvalid(), "Failed to create physics body for entity ID {}", p_entity.ID);

		return PhysicsSystemHandle(this, body_ID);
	}

	void PhysicsSystemJolt::destroy_body(const PhysicsSystemHandle& p_handle)
	{
		auto& body_interface = get_body_interface_no_lock();
		body_interface.RemoveBody(p_handle.m_jolt_body_ID);
		body_interface.DestroyBody(p_handle.m_jolt_body_ID);
	}

	void PhysicsSystemJolt::apply_impulse(const PhysicsSystemHandle& p_ID, const glm::vec3& p_force)
	{
		auto& body_interface = get_body_interface_no_lock();
		body_interface.AddImpulse(p_ID.m_jolt_body_ID, to_JPH(p_force));
	}
	glm::vec3 PhysicsSystemJolt::get_accumulated_force(const PhysicsSystemHandle& p_ID) const
	{
		return on_body_no_lock(p_ID.m_jolt_body_ID, [](JPH::Body& body) { return to_glm(body.GetAccumulatedForce()); });
	}
	glm::vec3 PhysicsSystemJolt::get_accumulated_torque(const PhysicsSystemHandle& p_ID) const
	{
		return on_body_no_lock(p_ID.m_jolt_body_ID, [](JPH::Body& body) { return to_glm(body.GetAccumulatedTorque()); });
	}

	void PhysicsSystemJolt::set_position(const PhysicsSystemHandle& p_ID, const glm::vec3& p_position)
	{
		auto& body_interface = get_body_interface_no_lock();
		body_interface.SetPosition(p_ID.m_jolt_body_ID, to_JPH(p_position), JPH::EActivation::DontActivate);
	}
	glm::vec3 PhysicsSystemJolt::get_position(const PhysicsSystemHandle& p_ID) const
	{
		auto& body_interface = get_body_interface_no_lock();
		return to_glm(body_interface.GetPosition(p_ID.m_jolt_body_ID));
	}

	void PhysicsSystemJolt::set_rotation(const PhysicsSystemHandle& p_ID, const glm::quat& p_rotation)
	{
		auto& body_interface = get_body_interface_no_lock();
		body_interface.SetRotation(p_ID.m_jolt_body_ID, to_JPH(p_rotation), JPH::EActivation::DontActivate);
	}
	glm::quat PhysicsSystemJolt::get_rotation(const PhysicsSystemHandle& p_ID) const
	{
		auto& body_interface = get_body_interface_no_lock();
		return to_glm(body_interface.GetRotation(p_ID.m_jolt_body_ID));
	}

	void PhysicsSystemJolt::set_velocity(const PhysicsSystemHandle& p_ID, const glm::vec3& p_velocity)
	{
		auto& body_interface = get_body_interface_no_lock();
		body_interface.SetLinearVelocity(p_ID.m_jolt_body_ID, to_JPH(p_velocity));
	}
	glm::vec3 PhysicsSystemJolt::get_velocity(const PhysicsSystemHandle& p_ID) const
	{
		auto& body_interface = get_body_interface_no_lock();
		return to_glm(body_interface.GetLinearVelocity(p_ID.m_jolt_body_ID));
	}
	glm::vec3 PhysicsSystemJolt::get_angular_velocity(const PhysicsSystemHandle& p_ID) const
	{
		return on_body_no_lock(p_ID.m_jolt_body_ID, [](JPH::Body& body) { return to_glm(body.GetAngularVelocity()); });
	}
	void PhysicsSystemJolt::set_angular_velocity(const PhysicsSystemHandle& p_ID, const glm::vec3& p_angular_velocity)
	{
		auto& body_interface = get_body_interface_no_lock();
		body_interface.SetAngularVelocity(p_ID.m_jolt_body_ID, to_JPH(p_angular_velocity));
	}

	void PhysicsSystemJolt::set_mass(const PhysicsSystemHandle& p_ID, float p_mass)
	{
		on_body_no_lock(p_ID.m_jolt_body_ID, [new_mass = p_mass](JPH::Body& body) { body.GetMotionProperties()->ScaleToMass(new_mass); });
	}
	float PhysicsSystemJolt::get_mass(const PhysicsSystemHandle& p_ID) const
	{
		return on_body_no_lock(p_ID.m_jolt_body_ID, [](JPH::Body& body) {
			float inv = body.GetMotionProperties()->GetInverseMass();
			if (inv <= 0.0f)
				return 0.0f;
			else
				return 1.0f / inv;
		});
	}

	void PhysicsSystemJolt::set_restitution(const PhysicsSystemHandle& p_ID, float p_restitution)
	{
		auto& body_interface = get_body_interface_no_lock();
		body_interface.SetRestitution(p_ID.m_jolt_body_ID, p_restitution);
	}
	float PhysicsSystemJolt::get_restitution(const PhysicsSystemHandle& p_ID) const
	{
		auto& body_interface = get_body_interface_no_lock();
		return body_interface.GetRestitution(p_ID.m_jolt_body_ID);
	}

	void PhysicsSystemJolt::set_gravity_factor(const PhysicsSystemHandle& p_ID, float p_gravity_factor)
	{
		auto& body_interface = get_body_interface_no_lock();
		body_interface.SetGravityFactor(p_ID.m_jolt_body_ID, p_gravity_factor);
	}
	float PhysicsSystemJolt::get_gravity_factor(const PhysicsSystemHandle& p_ID) const
	{
		auto& body_interface = get_body_interface_no_lock();
		return body_interface.GetGravityFactor(p_ID.m_jolt_body_ID);
	}

	void PhysicsSystemJolt::set_friction(const PhysicsSystemHandle& p_ID, float p_friction)
	{
		auto& body_interface = get_body_interface_no_lock();
		body_interface.SetFriction(p_ID.m_jolt_body_ID, p_friction);
	}
	float PhysicsSystemJolt::get_friction(const PhysicsSystemHandle& p_ID) const
	{
		auto& body_interface = get_body_interface_no_lock();
		return body_interface.GetFriction(p_ID.m_jolt_body_ID);
	}

	Geometry::AABB PhysicsSystemJolt::get_bounding_box() const
	{
		return to_geom(physics_system.GetBroadPhaseQuery().GetBounds());
	}
	Geometry::AABB PhysicsSystemJolt::get_bounding_box(const PhysicsSystemHandle& p_ID) const
	{
		Geometry::AABB AABB;
		on_body_no_lock(p_ID.m_jolt_body_ID, [&](JPH::Body& body) { AABB = to_geom(body.GetWorldSpaceBounds()); });
		return AABB;
	}

	std::optional<ECS::Entity> PhysicsSystemJolt::cast_ray(const Geometry::Ray& p_ray) const
	{
		// Jolt direction is a displacement to the endpoint, not a unit direction.
		// Scale the normalised direction to cover the full visible range.
		constexpr float ray_length = 10000.f;
		JPH::RRayCast ray{ to_JPH(p_ray.m_start), to_JPH(p_ray.m_direction) * ray_length };
		JPH::RayCastResult result;
		if (physics_system.GetNarrowPhaseQuery().CastRay(ray, result))
		{
			auto entID = on_body_no_lock(result.mBodyID, [&](JPH::Body& body) { return body.GetUserData(); });
			return ECS::Entity{ entID };
		}
		return std::nullopt;
	}

} // namespace System