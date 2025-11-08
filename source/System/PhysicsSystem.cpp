#include "PhysicsSystem.hpp"
#include "CollisionSystem.hpp"
#include "SceneSystem.hpp"

#include "Component/FirstPersonCamera.hpp"
#include "Component/RigidBody.hpp"
#include "Component/Transform.hpp"
#include "ECS/Storage.hpp"

#include "Geometry/Geometry.hpp"
#include "Utility/Performance.hpp"
#include "Utility/Utility.hpp"

#include "Jolt/Jolt.h"
#include "Jolt/RegisterTypes.h"
#include "Jolt/Core/Factory.h"
#include "Jolt/Core/TempAllocator.h"
#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Physics/PhysicsSettings.h"
#include "Jolt/Physics/PhysicsSystem.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/Physics/Collision/Shape/SphereShape.h"
#include "Jolt/Physics/Body/BodyCreationSettings.h"
#include "Jolt/Physics/Body/BodyActivationListener.h"

#include <thread>

namespace System
{
	PhysicsSystem::PhysicsSystem(SceneSystem& scene_system, CollisionSystem& collision_system)
		: m_update_count{0}
		, m_restitution{0.8f}
		, m_apply_collision_response{true}
		, m_bool_apply_kinematic{true}
		, m_scene_system{scene_system}
		, m_collision_system{collision_system}
		, m_total_simulation_time{DeltaTime::zero()}
		, m_gravity{glm::vec3(0.f, -9.81f, 0.f)}
	{
		init_jolt();
	}

	void PhysicsSystem::integrate(const DeltaTime& p_delta_time)
	{
		static_assert(std::is_same_v<DeltaTime, std::chrono::duration<float, std::ratio<1>>>, "PhysicsSystem::integrate expects DeltaTime to be a duration in seconds with float precision.");

		PERF(PhysicsSystemIntegrate);

		m_update_count++;
		m_total_simulation_time += p_delta_time;

		if (!m_bool_apply_kinematic)
			return;

		auto& scene = m_scene_system.get_current_scene_entities();
		scene.foreach([this, &p_delta_time, &scene](ECS::Entity& entity, Component::RigidBody& rigid_body, Component::Transform& transform)
		{
			if (rigid_body.m_apply_gravity)
				rigid_body.m_force += rigid_body.m_mass * m_gravity; // F = ma

			{ // Linear motion
				// Change in momentum is equal to the force = dp/dt = F
				const auto change_in_momentum = rigid_body.m_force * p_delta_time.count(); // dp = F dt
				rigid_body.m_momentum += change_in_momentum;

				// Convert momentum to velocity by dividing by mass: p = mv
				rigid_body.m_velocity = rigid_body.m_momentum / rigid_body.m_mass; // v = p/v

				// Integrate velocity to find new position: dx/dt = v
				const auto change_in_position = rigid_body.m_velocity * p_delta_time.count(); // dx = v dt
				transform.m_position += change_in_position;

				rigid_body.m_force = glm::vec3(0.f); // Reset back to 0 after applying the force on the body.
			}

			{ // Angular motion
				// http://physics.bu.edu/~redner/211-sp06/class-rigid-body/angularmo.html
				const auto change_in_angular_momentum = rigid_body.m_torque * p_delta_time.count(); // dL = T dt
				rigid_body.m_angular_momentum += change_in_angular_momentum;

				// Convert angular momentum to angular velocity by dividing by inertia tensor: L = Iω
				rigid_body.m_angular_velocity = rigid_body.m_angular_momentum / rigid_body.m_inertia_tensor; // ω = L / I

				// To integrate the new quat orientation we convert the angular velocity into quaternion form - spin.
				// Spin represents a time derivative of orientation. https://www.cs.cmu.edu/~baraff/sigcourse/notesd1.pdf
				const glm::quat spin = 0.5f * glm::quat(0.f, (rigid_body.m_angular_velocity * p_delta_time.count())) * transform.m_orientation;

				// Integrate spin to find the new orientation
				transform.m_orientation += spin;
				transform.m_orientation = glm::normalize(transform.m_orientation);
			}

			ECS::Entity collided_entity = ECS::Entity(0);
			if (auto collision = m_collision_system.get_collision(entity, &collided_entity))
			{
				if (m_apply_collision_response)
				{
					// A collision has occurred at the new position, the response depends on the collided entity having a rigibBody to apply a response to.
					// We already know the collided Entity has a Transform component from CollisionSystem::getCollision so we dont have to check it here.
					// The collision data returned is original-Entity-centric this convention is carried over in the response here when calling angular_impulse.
					if (scene.has_components<Component::RigidBody>(collided_entity))
					{
						auto& rigid_body_2 = m_scene_system.get_current_scene_entities().get_component<Component::RigidBody>(collided_entity);
						auto& transform_2  = m_scene_system.get_current_scene_entities().get_component<Component::Transform>(collided_entity);

						auto impulse = Geometry::angular_impulse(collision->position, collision->normal, m_restitution,
																transform.m_position, rigid_body.m_velocity, rigid_body.m_angular_velocity, rigid_body.m_mass, rigid_body.m_inertia_tensor,
																transform_2.m_position, rigid_body_2.m_velocity, rigid_body_2.m_angular_velocity, rigid_body_2.m_mass, rigid_body_2.m_inertia_tensor);

						const auto r             = collision->position - transform.m_position;
						const auto inverseTensor = glm::inverse(rigid_body.m_inertia_tensor);

						rigid_body.m_velocity        = rigid_body.m_velocity + (impulse / rigid_body.m_mass);
						rigid_body.m_angular_velocity = rigid_body.m_angular_velocity + (glm::cross(r, impulse) * inverseTensor);

						// #TODO: Apply a response to collision.mEntity
					}
				}
			}
		});
	}


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

	void PhysicsSystem::init_jolt()
	{
		JPH::RegisterDefaultAllocator();

		JPH::Trace = [](const char *inFMT, ...) {
			// Format the message
			va_list list;
			va_start(list, inFMT);
			char buffer[1024];
			vsnprintf(buffer, sizeof(buffer), inFMT, list);
			va_end(list);
			LOG("[JOLT] {}", buffer);
		};

		JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = [](const char *inExpression, const char *inMessage, const char *inFile, unsigned int inLine) {
			LOG_ERROR(false, "[JOLT] Assertion failed in file {} at line {}: ({}) {}", inFile, inLine, inExpression, (inMessage != nullptr ? inMessage : ""));
			return true;
		});

		JPH::Factory::sInstance = new JPH::Factory();
		// If you have your own custom shape types you probably need to register their handlers with the CollisionDispatch before calling this function.
		// If you implement your own default material (PhysicsMaterial::sDefault) make sure to initialize it before this function or else this function will create one for you.
		JPH::RegisterTypes();
		constexpr size_t temp_allocator_size = 10 * 1024 * 1024; // 10 MB
		JPH::TempAllocatorImpl temp_allocator(temp_allocator_size);

		constexpr int cMaxPhysicsJobs = 2048;
		constexpr int cMaxPhysicsBarriers = 8;
		// We need a job system that will execute physics jobs on multiple threads.
		// TODO: Implement a JobSystem interface that integrates with Jolt
		// JobSystemThreadPool is an example implementation.
		JPH::JobSystemThreadPool job_system(cMaxPhysicsJobs, cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

		constexpr uint32_t cMaxBodies = 65536;
		constexpr uint32_t cNumBodyMutexes = 0; // Determines how many mutexes to allocate to protect rigid bodies from concurrent access default 0.
		// Max number of body pairs to be queued at any time, the broad phase will detect overlapping
		// based on bounding boxes and queue them up to be processed by the narrow phase.
		// If this number is too low the queue will fill up and the broad phase jobs will start to do narrow phase work (less efficient).
		constexpr uint32_t cMaxBodyPairs = 65536;
		// Maximum size of the contact constraint buffer. If more contacts between bodies are detected than this
		// then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
		constexpr uint32_t cMaxContactConstraints = 10240;

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
		physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);

		// A body activation listener gets notified when bodies activate and go to sleep
		// Note that this is called from a job so whatever you do here needs to be thread safe.
		// Registering one is entirely optional.
		//MyBodyActivationListener body_activation_listener;
		//physics_system.SetBodyActivationListener(&body_activation_listener);
		// A contact listener gets notified when bodies (are about to) collide, and when they separate again.
		// Note that this is called from a job so whatever you do here needs to be thread safe.
		// Registering one is entirely optional.
		//MyContactListener contact_listener;
		//physics_system.SetContactListener(&contact_listener);

		// The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
		// variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
		auto& body_interface = physics_system.GetBodyInterface();

		using namespace JPH::literals; // _r

		// Next we can create a rigid body to serve as the floor, we make a large box
		// Create the settings for the collision volume (the shape).
		// Note that for simple shapes (like boxes) you can also directly construct a BoxShape.
		JPH::BoxShapeSettings floor_shape_settings(JPH::Vec3(100.0f, 1.0f, 100.0f));
		floor_shape_settings.SetEmbedded(); // A ref counted object on the stack (base class RefTarget) should be marked as such to prevent it from being freed when its reference count goes to 0.
		// Create the shape
		JPH::ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
		JPH::ShapeRefC floor_shape = floor_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()
		// Create the settings for the body itself. Note that here you can also set other properties like the restitution / friction.
		JPH::BodyCreationSettings floor_settings(floor_shape, JPH::RVec3(0.0_r, -1.0_r, 0.0_r), JPH::Quat::sIdentity(), JPH::EMotionType::Static, Layers::NON_MOVING);
		JPH::Body* floor = body_interface.CreateBody(floor_settings); // Note that if we run out of bodies this can return nullptr
		// Add it to the world
		body_interface.AddBody(floor->GetID(), JPH::EActivation::DontActivate);



		// Now create a dynamic body to bounce on the floor
		// Note that this uses the shorthand version of creating and adding a body to the world
		JPH::BodyCreationSettings sphere_settings(new JPH::SphereShape(0.5f), JPH::RVec3(0.0_r, 2.0_r, 0.0_r), JPH::Quat::sIdentity(), JPH::EMotionType::Dynamic, Layers::MOVING);
		JPH::BodyID sphere_id = body_interface.CreateAndAddBody(sphere_settings, JPH::EActivation::Activate);
		// Now you can interact with the dynamic body, in this case we're going to give it a velocity.
		// (note that if we had used CreateBody then we could have set the velocity straight on the body before adding it to the physics system)
		body_interface.SetLinearVelocity(sphere_id, JPH::Vec3(0.0f, -5.0f, 0.0f));


		// Optional step: Before starting the physics simulation you can optimize the broad phase. This improves collision detection performance (it's pointless here because we only have 2 bodies).
		// You should definitely not call this every frame or when e.g. streaming in a new level section as it is an expensive operation.
		// Instead insert all new objects in batches instead of 1 at a time to keep the broad phase efficient.
		physics_system.OptimizeBroadPhase();




		// Now we're ready to simulate the body, keep simulating until it goes to sleep
		uint32_t step = 0;
		const float cDeltaTime = 1.0f / 60.0f;
		while (body_interface.IsActive(sphere_id))
		{
			// Next step
			++step;

			// Output current position and velocity of the sphere
			JPH::RVec3 position = body_interface.GetCenterOfMassPosition(sphere_id);
			JPH::Vec3 velocity = body_interface.GetLinearVelocity(sphere_id);
			LOG("Step {}: Position = ({}, {}, {}), Velocity = ({}, {}, {})", step, position.GetX(), position.GetY(), position.GetZ(), velocity.GetX(), velocity.GetY(), velocity.GetZ());

			// If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
			const int cCollisionSteps = 1;

			// Step the world
			physics_system.Update(cDeltaTime, cCollisionSteps, &temp_allocator, &job_system);
		}


		{// Cleanup
			// Remove the sphere from the physics system. Note that the sphere itself keeps all of its state and can be re-added at any time.
			body_interface.RemoveBody(sphere_id);

			// Destroy the sphere. After this the sphere ID is no longer valid.
			body_interface.DestroyBody(sphere_id);

			// Remove and destroy the floor
			body_interface.RemoveBody(floor->GetID());
			body_interface.DestroyBody(floor->GetID());

			// Unregisters all types with the factory and cleans up the default material
			JPH::UnregisterTypes();

			// Destroy the factory
			delete JPH::Factory::sInstance;
			JPH::Factory::sInstance = nullptr;
		}
	}
} // namespace System