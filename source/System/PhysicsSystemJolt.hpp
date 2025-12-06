#pragma once

#include "IPhysicsSystem.hpp"

#include "Jolt/Jolt.h"
#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Core/TempAllocator.h"
#include "Jolt/Physics/PhysicsSystem.h"

#include "Utility/Logger.hpp"

namespace System
{
	class SceneSystem;

	// Layer that objects can be in, determines which other objects it can collide with
	// Typically you at least want to have 1 layer for moving bodies and 1 layer for static bodies, but you can have more
	// layers if you want. E.g. you could have a layer for high detail collision (which is not used by the physics simulation
	// but only if you do collision testing).
	namespace Layers
	{
		static constexpr JPH::ObjectLayer NON_MOVING = 0;
		static constexpr JPH::ObjectLayer MOVING     = 1;
		static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
	};
	// Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
	// a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
	// You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
	// many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
	// your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported.
	namespace BroadPhaseLayers
	{
		static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
		static constexpr JPH::BroadPhaseLayer MOVING(1);
		static constexpr uint32_t NUM_LAYERS(2);
	};

	// This defines a mapping between object and broadphase layers.
	class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
	{
		JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
	public:
		BPLayerInterfaceImpl()
		{
			// Create a mapping table from object to broad phase layer
			mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
			mObjectToBroadPhase[Layers::MOVING]     = BroadPhaseLayers::MOVING;
		}

		virtual uint32_t GetNumBroadPhaseLayers() const override { return BroadPhaseLayers::NUM_LAYERS; }

		virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
		{
			JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
			return mObjectToBroadPhase[inLayer];
		}

	#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
		virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
		{
			switch ((JPH::BroadPhaseLayer::Type)inLayer)
			{
				case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING: return "NON_MOVING";
				case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING: return "MOVING";
				default: JPH_ASSERT(false); return "INVALID";
			}
		}
	#endif
	};

	// Class that determines if an object layer can collide with a broadphase layer
	class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
	{
	public:
		virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
		{
			switch (inLayer1)
			{
				case Layers::NON_MOVING: return inLayer2 == BroadPhaseLayers::MOVING;
				case Layers::MOVING:     return true;
				default:
					JPH_ASSERT(false);
					return false;
			}
		}
	};
	// Class that determines if two object layers can collide
	class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
	{
	public:
		virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
		{
			switch (inObject1)
			{
				case Layers::NON_MOVING: return inObject2 == Layers::MOVING; // Non moving only collides with moving
				case Layers::MOVING:     return true; // Moving collides with everything
				default:
					JPH_ASSERT(false);
					return false;
			}
		}
	};



	// A numerical integrator, PhysicsSystem take Transform and RigidBody components and applies kinematic equations.
	// The system is force based and numerically integrates
	class PhysicsSystemJolt : public IPhysicsSystem
	{
		// Ensures Jolt's process-wide data is initialized before any Jolt objects are created.
		struct JoltInitWrapper { JoltInitWrapper(); };
		JoltInitWrapper jolt_init;


		SceneSystem& m_scene_system;

		unsigned int max_physics_jobs;
		unsigned int max_physics_barriers;
		uint32_t max_bodies;
		// Determines how many mutexes to allocate to protect rigid bodies from concurrent access default 0.
		uint32_t num_body_mutexes;
		// Max number of body pairs to be queued at any time, the broad phase will detect overlapping
		// based on bounding boxes and queue them up to be processed by the narrow phase.
		// If this number is too low the queue will fill up and the broad phase jobs will start to do narrow phase work (less efficient).
		uint32_t max_body_pairs;
		// Maximum size of the contact constraint buffer. If more contacts between bodies are detected than this
		// then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
		uint32_t max_contact_constraints;
		size_t temp_allocator_size;


		JPH::TempAllocatorImpl temp_allocator;
		// TODO: Implement a JobSystem interface that integrates with Jolt, JobSystemThreadPool is an example implementation.
		JPH::JobSystemThreadPool job_system;

		// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
		// Also have a look at BroadPhaseLayerInterfaceTable or BroadPhaseLayerInterfaceMask for a simpler interface.
		BPLayerInterfaceImpl broad_phase_layer_interface;
		// Create class that filters object vs broadphase layers
		// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
		// Also have a look at ObjectVsBroadPhaseLayerFilterTable or ObjectVsBroadPhaseLayerFilterMask for a simpler interface.
		ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;

		// Create class that filters object vs object layers
		// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
		// Also have a look at ObjectLayerPairFilterTable or ObjectLayerPairFilterMask for a simpler interface.
		ObjectLayerPairFilterImpl object_vs_object_layer_filter;

		JPH::PhysicsSystem physics_system;

	public:
		PhysicsSystemJolt(SceneSystem& scene_system);
		~PhysicsSystemJolt();

		void update(const DeltaTime& p_delta_time) override;

		[[nodiscard]] PhysicsSystemHandle create_body(const BodySettings& p_body_settings, const ECS::Entity& p_entity) override;
		void destroy_body(const PhysicsSystemHandle& p_handle) override;

		void apply_impulse(const PhysicsSystemHandle& p_ID, const glm::vec3& p_force) override;
		glm::vec3 get_accumulated_force(const PhysicsSystemHandle& p_ID) const override;
		glm::vec3 get_accumulated_torque(const PhysicsSystemHandle& p_ID) const override;


		void set_position(const PhysicsSystemHandle& p_ID, const glm::vec3& p_position) override;
		glm::vec3 get_position(const PhysicsSystemHandle& p_ID) const                   override;
		void set_rotation(const PhysicsSystemHandle& p_ID, const glm::quat& p_rotation) override;
		glm::quat get_rotation(const PhysicsSystemHandle& p_ID) const                   override;

		void set_velocity(const PhysicsSystemHandle& p_ID, const glm::vec3& p_velocity) override;
		glm::vec3 get_velocity(const PhysicsSystemHandle& p_ID) const                   override;
		void set_angular_velocity(const PhysicsSystemHandle& p_ID, const glm::vec3& p_angular_velocity) override;
		glm::vec3 get_angular_velocity(const PhysicsSystemHandle& p_ID) const override;

		void set_mass(const PhysicsSystemHandle& p_ID, float p_mass) override;
		float get_mass(const PhysicsSystemHandle& p_ID) const override;

		void set_restitution(const PhysicsSystemHandle& p_ID, float p_restitution) override;
		float get_restitution(const PhysicsSystemHandle& p_ID) const override;

		void set_gravity_factor(const PhysicsSystemHandle& p_ID, float p_gravity_factor) override;
		float get_gravity_factor(const PhysicsSystemHandle& p_ID) const override;

		void set_friction(const PhysicsSystemHandle& p_ID, float p_friction) override;
		float get_friction(const PhysicsSystemHandle& p_ID) const override;


		Geometry::AABB get_bounding_box() const override;
		Geometry::AABB get_bounding_box(const PhysicsSystemHandle& p_ID) const override;

		std::optional<ECS::Entity> cast_ray(const Geometry::Ray& p_ray) const override;

	private:
		template<typename Func>
		std::invoke_result_t<Func, JPH::Body&> on_body_locked(JPH::BodyID ID, Func&& func) const
		{
			JPH::BodyLockRead lock(physics_system.GetBodyLockInterface(), ID);
			if (lock.Succeeded())
				return func(lock.GetBody());
			else
				ASSERT_FAIL("Body not found in on_body_locked");
		}
		template<typename Func>
		std::invoke_result_t<Func, JPH::Body&> on_body_no_lock(JPH::BodyID ID, Func&& func) const
		{
			auto& no_lock_interface = physics_system.GetBodyLockInterfaceNoLock();
			if (JPH::Body* body = no_lock_interface.TryGetBody(ID))
				return func(*body);
			else
				ASSERT_FAIL("Body not found in on_body_no_lock");
		}

		JPH::BodyInterface& get_body_interface_locked()  { return physics_system.GetBodyInterface(); }
		JPH::BodyInterface& get_body_interface_no_lock() { return physics_system.GetBodyInterfaceNoLock(); }
		const JPH::BodyInterface& get_body_interface_locked() const  { return physics_system.GetBodyInterface(); }
		const JPH::BodyInterface& get_body_interface_no_lock() const { return physics_system.GetBodyInterfaceNoLock(); }
	};

} // namespace System