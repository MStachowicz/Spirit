#pragma once

#include "Entity.hpp"
#include "Meta.hpp"

#include "Utility/Logger.hpp"

#include <array>
#include <bitset>
#include <cstdint>
#include <fstream>
#include <optional>
#include <type_traits>

namespace ECS
{
	constexpr size_t Max_Component_Count = 32;
	using ComponentID     = size_t; // Unique identifier for any type passed into ECSStorage.
	using ComponentBitset = std::bitset<Max_Component_Count>;

	// Stores per ComponentType information ECS needs after type erasure.
	class ComponentData
	{
	public:
		// Forward declare the ComponentData constructor to allow for the ComponentData constructor to be defined after the Component class.
		template <typename ComponentType>
		ComponentData(Meta::PackArg<ComponentType>);

		ComponentID ID; // Unique ID/index of the Type. Corresponds to the index in the ComponentRegister::type_infos vector.
		size_t size;    // sizeof of the Type
		size_t align;   // alignof of the type
		// Call the destructor of the object at p_address_to_destroy.
		void (*Destruct)(void* p_address_to_destroy);
		// move-assign the object pointed to by p_source_address into the memory pointed to by p_destination_address.
		void (*MoveAssign)(void* p_destination_address, void* p_source_address);
		// placement-new move-construct the object pointed to by p_source_address into the memory pointed to by p_destination_address.
		void (*MoveConstruct)(void* p_destination_address, void* p_source_address);
		// Serialise the object into p_file.
		void (*Serialise)(void* p_address, std::ofstream& p_file);
		// Deserialise the object into p_file.
		void (*Deserialise)(void* p_address, std::ifstream& p_file);
	};

	class Component
	{
		static inline std::array<std::optional<ComponentData>, Max_Component_Count> type_infos = {};

	public:

		template <typename ComponentType>
		static inline ComponentID get_ID() // todo add consteval here
		{
			using Type = std::decay_t<ComponentType>;
			//static_assert(Has_Persistent_ID<Type>, "Component does not have a persistent_ID member. Add a static constexpr ComponentID persistent_ID member to the class.");
			return std::decay_t<Type>::Persistent_ID;
		}

		// Called once per ComponentType to store the ComponentData. Must be called before any other ECS functions.
		template <typename ComponentType>
		static inline void set_info()
		{
			ASSERT(type_infos[get_ID<ComponentType>()] == std::nullopt, "Component already registered. Call set_info only once per ComponentType or check for duplicate Persistent_ID values across ComponentsTypes.");
			ASSERT(get_ID<ComponentType>() < Max_Component_Count, "Component ID out of bounds. Increase Max_Component_Count.");

			type_infos[get_ID<ComponentType>()] = ComponentData(Meta::PackArg<ComponentType>());
		}

		// Get the ComponentData given a ComponentID.
		static inline const ComponentData& get_info(ComponentID p_component_ID)
		{
			ASSERT(type_infos[p_component_ID].has_value(), "Component not registered. Call set_info for the ComponentType before using it.");

			return *type_infos[p_component_ID];
		}

		// Generates a bitset out of all the ComponentTypes. Skips over Entity params.
		template <typename... ComponentTypes>
		static inline ComponentBitset get_component_bitset()
		{
			ComponentBitset componentBitset;

			auto setComponentBit = [&componentBitset]<typename ComponentType>()
			{
				if constexpr (!std::is_same_v<Entity, std::decay_t<ComponentType>>) // Ignore any Entity params supplied.
					componentBitset.set(get_ID<ComponentType>());
			};
			(setComponentBit.template operator()<ComponentTypes>(), ...);

			return componentBitset;
		}
	};

	// Construct the ComponentData for a ComponentType.
	template <typename ComponentType>
	ComponentData::ComponentData(Meta::PackArg<ComponentType>)
		: ID{Component::get_ID<ComponentType>()}
		, size{sizeof(std::decay_t<ComponentType>)}
		, align{alignof(std::decay_t<ComponentType>)}
		, Destruct{[](void* p_address)
		{
			using Type = std::decay_t<ComponentType>;
			static_cast<Type*>(p_address)->~Type();
		}}
		, MoveAssign{[](void* p_destination_address, void* p_source_address)
		{
			using Type                                 = std::decay_t<ComponentType>;
			*static_cast<Type*>(p_destination_address) = std::move(*static_cast<Type*>(p_source_address));
		}}
		, MoveConstruct{[](void* p_destination_address, void* p_source_address)
		{
			using Type = std::decay_t<ComponentType>;
			new (p_destination_address) Type(std::move(*static_cast<Type*>(p_source_address)));
		}}
		, Serialise{[](void* p_address, std::ofstream& p_file)
		{
			using Type = std::decay_t<ComponentType>;
			Type::serialise(*static_cast<Type*>(p_address), p_file);
		}}
		, Deserialise{[](void* p_address, std::ifstream& p_file)
		{
			using Type                     = std::decay_t<ComponentType>;
			*static_cast<Type*>(p_address) = Type::deserialise(p_file);
		}}
	{}

} // namespace ECS