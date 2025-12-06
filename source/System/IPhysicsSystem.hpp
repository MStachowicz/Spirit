#pragma once

#include "ECS/Entity.hpp"

#include "Geometry/AABB.hpp"
#include "Geometry/Shape.hpp"

#include "Utility/Config.hpp"

#include "glm/gtc/quaternion.hpp"
#include "glm/vec3.hpp"

#include "Jolt/Jolt.h"
#include "Jolt/Physics/Body/BodyID.h"

#include <optional>
#include <vector>

namespace Geometry { class Ray; }
namespace System
{
	// Settings defining the properties and behavior of a generalised physics body.
	struct BodySettings
	{
		enum class MotionType : uint8_t
		{
			Static,    // Immovable object, forces and velocities are ignored.
			Kinematic, // Movable using velocities only, does not respond to forces.
			Dynamic    // Responds to forces as a normal physics object.
		};
		enum class Type : uint8_t { Rigid, SoftBody };

		Geometry::Shape m_shape;
		glm::quat m_rotation;
		MotionType m_motion_type;
		Type m_type;

		BodySettings(const Geometry::Shape& p_shape, const glm::quat& p_rotation = glm::identity<glm::quat>(), MotionType p_motion_type = MotionType::Dynamic)
			: m_shape{p_shape} , m_rotation{p_rotation} , m_motion_type{p_motion_type} , m_type{Type::Rigid}
		{}
	};

	class IPhysicsSystem;
	// Holds a reference to the physics system and the ID into that system.
	// IPhysicsSystem assign these via create_body calls on update.
	struct PhysicsSystemHandle
	{
		IPhysicsSystem* m_system;
		JPH::BodyID m_jolt_body_ID;
	};

	// Interface for physics system ECS can interface with.
	class IPhysicsSystem
	{
	public:
		bool m_pause_simulation = false;
		size_t m_update_count;

		IPhysicsSystem()
			: m_pause_simulation{false}
			, m_update_count{0}
		{}
		virtual ~IPhysicsSystem() = default;

		virtual void update(const DeltaTime& p_delta_time) = 0;

		// Factory methods
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		[[nodiscard]] virtual PhysicsSystemHandle create_body(const BodySettings& p_body_settings, const ECS::Entity& p_entity) = 0;
		virtual void destroy_body(const PhysicsSystemHandle& p_handle)               = 0;

		// Getters and Setters
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// Apply a linear force to the body with the given ID.
		virtual void apply_impulse(const PhysicsSystemHandle& p_ID, const glm::vec3& p_force) = 0;
		virtual glm::vec3 get_accumulated_force(const PhysicsSystemHandle& p_ID) const        = 0;
		virtual glm::vec3 get_accumulated_torque(const PhysicsSystemHandle& p_ID) const       = 0;

		virtual void set_position(const PhysicsSystemHandle& p_ID, const glm::vec3& p_position) = 0;
		virtual glm::vec3 get_position(const PhysicsSystemHandle& p_ID) const                   = 0;

		virtual void set_rotation(const PhysicsSystemHandle& p_ID, const glm::quat& p_rotation) = 0;
		virtual glm::quat get_rotation(const PhysicsSystemHandle& p_ID) const                   = 0;

		// Set linear velocity (meters per second).
		virtual void set_velocity(const PhysicsSystemHandle& p_ID, const glm::vec3& p_velocity) = 0;
		// Get linear velocity (meters per second).
		virtual glm::vec3 get_velocity(const PhysicsSystemHandle& p_ID) const                   = 0;

		// Set angular velocity (radians per second).
		virtual void set_angular_velocity(const PhysicsSystemHandle& p_ID, const glm::vec3& p_angular_velocity) = 0;
		// Get angular velocity (radians per second).
		virtual glm::vec3 get_angular_velocity(const PhysicsSystemHandle& p_ID) const                           = 0;

		virtual void set_mass(const PhysicsSystemHandle& p_ID, float p_mass) = 0;
		virtual float get_mass(const PhysicsSystemHandle& p_ID) const        = 0;

		// 0 = completely inelastic collision response, 1 = completely elastic collision response
		virtual void set_restitution(const PhysicsSystemHandle& p_ID, float p_restitution) = 0;
		virtual float get_restitution(const PhysicsSystemHandle& p_ID) const               = 0;

		virtual void set_gravity_factor(const PhysicsSystemHandle& p_ID, float p_gravity_factor) = 0;
		virtual float get_gravity_factor(const PhysicsSystemHandle& p_ID) const                  = 0;

		virtual void set_friction(const PhysicsSystemHandle& p_ID, float p_friction) = 0;
		virtual float get_friction(const PhysicsSystemHandle& p_ID) const            = 0;

		// Utilities
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Get the bounds of the body with the given ID.
		virtual Geometry::AABB get_bounding_box(const PhysicsSystemHandle& p_ID) const = 0;
		// Get the bounds of the entire physics system.
		virtual Geometry::AABB get_bounding_box() const                                = 0;

		virtual std::optional<ECS::Entity> cast_ray(const Geometry::Ray& p_ray) const = 0;
	};
} // namespace System