#pragma once

#include "System/IPhysicsSystem.hpp"

#include "Geometry/Sphere.hpp"
#include "Geometry/AABB.hpp"

#include "glm/vec3.hpp"
#include "glm/mat3x3.hpp"

#include <iostream>
#include <optional>

namespace Component
{
	// Collider represents a handle to a physics body in the IPhysicsSystem.
	// It stores the BodyID assigned by the physics system and the cached body settings.
	class Collider
	{
	public:
		constexpr static size_t Persistent_ID = 4;

		// Assigned by the physics system when it next updates.
		std::optional<System::PhysicsSystemHandle> m_physics_system_handle;
		// Settings used to construct the body.
		std::optional<System::BodySettings> m_body_settings_cache;

		Collider(const System::BodySettings& p_body_settings)
			: m_physics_system_handle{std::nullopt}
			, m_body_settings_cache{p_body_settings}
		{}
		Collider(const Geometry::Shape& p_shape, System::BodySettings::MotionType p_motion_type = System::BodySettings::MotionType::Dynamic)
			: m_physics_system_handle{std::nullopt}
			, m_body_settings_cache{System::BodySettings{p_shape, glm::identity<glm::quat>(), p_motion_type}}
		{}
		~Collider()
		{
			if (m_physics_system_handle)
				m_physics_system_handle->m_system->destroy_body(*m_physics_system_handle);
		}

		// Move constructor: transfer ownership, prevent double-destroy.
		Collider(Collider&& p_other) noexcept
			: m_physics_system_handle{std::exchange(p_other.m_physics_system_handle, std::nullopt)}
			, m_body_settings_cache{std::move(p_other.m_body_settings_cache)}
		{}
		// Move assignment: destroy our current body if any, then take ownership.
		Collider& operator=(Collider&& p_other) noexcept
		{
			if (this != &p_other)
			{
				if (m_physics_system_handle)
					m_physics_system_handle->m_system->destroy_body(*m_physics_system_handle);

				m_physics_system_handle = std::exchange(p_other.m_physics_system_handle, std::nullopt);
				m_body_settings_cache   = std::move(p_other.m_body_settings_cache);
			}
			return *this;
		}

		// Copying a Collider does NOT copy the runtime Jolt body — the copy gets a fresh settings cache to be registered on next update.
		Collider(const Collider& p_other)
			: m_physics_system_handle{std::nullopt}
			, m_body_settings_cache{p_other.m_body_settings_cache}
		{}
		Collider& operator=(const Collider& p_other)
		{
			if (this != &p_other)
			{
				if (m_physics_system_handle)
					m_physics_system_handle->m_system->destroy_body(*m_physics_system_handle);

				m_physics_system_handle = std::nullopt;
				m_body_settings_cache   = p_other.m_body_settings_cache;
			}
			return *this;
		}

		void draw_UI();
		auto get_Jolt_ID() const { return m_physics_system_handle->m_jolt_body_ID; }

		void set_position(const glm::vec3& p_position) { m_physics_system_handle->m_system->set_position(*m_physics_system_handle, p_position); }
		glm::vec3 get_position() const                 { return m_physics_system_handle->m_system->get_position(*m_physics_system_handle); }

		void set_rotation(const glm::quat& p_rotation) { m_physics_system_handle->m_system->set_rotation(*m_physics_system_handle, p_rotation); }
		glm::quat get_rotation() const                 { return m_physics_system_handle->m_system->get_rotation(*m_physics_system_handle); }

		void set_velocity(const glm::vec3& p_velocity) { m_physics_system_handle->m_system->set_velocity(*m_physics_system_handle, p_velocity); }
		glm::vec3 get_velocity() const                 { return m_physics_system_handle->m_system->get_velocity(*m_physics_system_handle); }

		void set_angular_velocity(const glm::vec3& p_angular_velocity) { m_physics_system_handle->m_system->set_angular_velocity(*m_physics_system_handle, p_angular_velocity); }
		glm::vec3 get_angular_velocity() const                         { return m_physics_system_handle->m_system->get_angular_velocity(*m_physics_system_handle); }

		void set_mass(float p_mass) { m_physics_system_handle->m_system->set_mass(*m_physics_system_handle, p_mass); }
		float get_mass() const      { return m_physics_system_handle->m_system->get_mass(*m_physics_system_handle); }

		float get_restitution() const { return m_physics_system_handle->m_system->get_restitution(*m_physics_system_handle); }
		void set_restitution(float p_restitution) { m_physics_system_handle->m_system->set_restitution(*m_physics_system_handle, p_restitution); }

		float get_friction() const { return m_physics_system_handle->m_system->get_friction(*m_physics_system_handle); }
		void set_friction(float p_friction) { m_physics_system_handle->m_system->set_friction(*m_physics_system_handle, p_friction); }

		float get_gravity_factor() const { return m_physics_system_handle->m_system->get_gravity_factor(*m_physics_system_handle); }
		void set_gravity_factor(float p_gravity_factor) { m_physics_system_handle->m_system->set_gravity_factor(*m_physics_system_handle, p_gravity_factor); }

		Geometry::AABB get_bounds() const { return m_physics_system_handle->m_system->get_bounding_box(*m_physics_system_handle); }

		// void set_scale(const glm::vec3& p_scale) { m_physics_system_handle->m_system->set_scale(*m_physics_system_handle, p_scale); }
		// glm::vec3 get_scale() const              { return m_physics_system_handle->m_system->get_scale(*m_physics_system_handle); }

		void apply_force(const glm::vec3& p_force) { m_physics_system_handle->m_system->apply_impulse(*m_physics_system_handle, p_force); }

		static void serialise(std::ostream& p_out, uint16_t p_version, const Collider& p_collider);
		static Collider deserialise(std::istream& p_in, uint16_t p_version);
	};
} // namespace Component