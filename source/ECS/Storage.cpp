#include "Storage.hpp"

#include "Utility/Serialise.hpp"

namespace ECS
{
	// Type definitions for the saving and loading of the storage.
	// If these types change or missmatch, saving/loading will break and needs to be handled using p_version.

	using Archetype_Count_t = uint64_t;
	using Entity_Count_t    = uint64_t;
	using Component_Count_t = uint64_t;
	using ComponentID_t     = uint8_t;

	static_assert(std::is_same<std::vector<int>::size_type, Archetype_Count_t>::value, "Archetype_Count_t doesn't match Vector::size_type. Update save/load type used.");
	static_assert(std::is_same<std::vector<int>::size_type, Entity_Count_t>::value,    "Entity_Count_t doesn't match Vector::size_type. Update save/load type used.");
	static_assert(std::is_same<std::vector<int>::size_type, Component_Count_t>::value, "Component_Count_t doesn't match Vector::size_type. Update save/load type used.");
	static_assert(std::is_same<ComponentID_t, ComponentID_t>::value,                   "ComponentID_t doesn't match ComponentID. Update save/load type used.");

	void Storage::Serialise(const Storage& p_storage, std::ofstream& p_out, uint16_t p_version)
	{
		//{ECS::Storage save format
		//uint16_t : archetypes count (only serialisable ones with entity count > 0 are saved)
		//	{Start Archetype
		//		uint32_t : entity/element count (always non-zero)
		//		uint16_t : component count (always non-zero)
		//		uint16_t : componentIDs per entity (only serialisable components)
		//		{Start Entity
		//			// Serialise each serialisable component in the entity.
		//		}End Entity
		//	}End Archetype
		//}

		// When saving archetypes we only save ones that have entities all their components are serialisable.
		// This means we can assume the archetypes are valid and avoid checking on deserialise.

		// Lambda to check if an archetype should be saved, depending on if it has entities and any serialisable components.
		auto should_save = [](const Archetype& p_archetype) { return !p_archetype.m_entities.empty() && p_archetype.m_is_serialisable; };

		Archetype_Count_t archetype_count = std::count_if(p_storage.m_archetypes.begin(), p_storage.m_archetypes.end(), [&](const Archetype& p_archetype)
			{ return should_save(p_archetype); });

		Utility::write_binary(p_out, archetype_count); // Even if there are no archetypes to save, we still need to save the count to deserialise correctly.
		if (archetype_count == 0) // No archetypes to deserialise, return early.
			return;

		for (const auto& archetype : p_storage.m_archetypes)
		{
			if (!should_save(archetype))
				continue;

			Entity_Count_t entity_count = archetype.m_entities.size();
			Utility::write_binary(p_out, entity_count);

			// Count the number of serialisable components in the archetype.
			Component_Count_t component_count = archetype.m_components.size();
			Utility::write_binary(p_out, component_count);

			// Save the ComponentIDs of the serialisable components in the archetype.
			for (const auto& component_layout : archetype.m_components)
			{
				ASSERT(component_layout.type_info.is_serialisable, "Only serialisable components should be saved.");

				ComponentID_t component_ID = component_layout.type_info.ID;
				Utility::write_binary(p_out, component_ID);
			}

			// Save the entities in the archetype.
			for (Entity_Count_t i = 0; i < entity_count; ++i)
			{
				auto index_start_pos = archetype.m_instance_size * i; // Position of the start of the instance at p_instance_index.

				for (const auto& component_layout : archetype.m_components)
				{
					BufferPosition component_start_pos = index_start_pos + component_layout.offset;
					component_layout.type_info.Serialise(&archetype.m_data[component_start_pos], p_out, p_version);
				}
			}
		}
	}

	Storage Storage::Deserialise(std::ifstream& p_in, uint16_t p_version)
	{
		//{ECS::Storage save format
		//uint16_t : archetypes to load (always non-zero)
		//	{Start Archetype
		//		uint32_t : entity/element count (always non-zero)
		//		uint16_t : component count (always non-zero)
		//		uint16_t : componentIDs per entity
		//		{Start Entity
		//			// Deserialise each component in the entity.
		//		}End Entity
		//	}End Archetype
		//}

		// Because we only save archetypes with entities and serialisable components, we can assume they are valid and avoid checking.

		Storage storage;

		Archetype_Count_t archetype_count;
		Utility::read_binary(p_in, archetype_count);

		// No archetypes to deserialise, return early.
		if (archetype_count == 0)
			return storage;

		storage.m_archetypes.reserve(archetype_count);

		for (Archetype_Count_t i = 0; i < archetype_count; ++i)
		{
			Entity_Count_t entity_count; // Number of entities in the archetype.
			Utility::read_binary(p_in, entity_count);

			Component_Count_t component_count; // Number of components in the archetype.
			Utility::read_binary(p_in, component_count);

			ComponentBitset component_bitset; // Bitset of the components in the archetype.
			// Used to keep the order of the components. We need to know the order of the components in the file to Deserialise them correctly.
			std::vector<ComponentID_t> component_IDs;
			component_IDs.reserve(component_count);

			for (Component_Count_t j = 0; j < component_count; ++j)
			{
				ComponentID_t component_ID;
				Utility::read_binary(p_in, component_ID);
				component_bitset.set(component_ID);
				component_IDs.push_back(component_ID);
			}

			ArchetypeID archetype_ID = storage.m_archetypes.size();
			auto& archetype = storage.m_archetypes.emplace_back(component_bitset);
			// Reserve enough size for entity_count entities.
			archetype.reserve(next_greater_power_of_2(entity_count));

			// If ECS::get_component_layout has changed, the order of the components in the Archetype may not match the order saved in the file.
			// Create a vector of ComponentLayouts that matches the order of the components in the file to ensure they are deserialised correctly.
			std::vector<ComponentLayout> components;
			components.reserve(component_count);
			for (const auto& component_ID : component_IDs)
				components.push_back(archetype.get_component_layout(component_ID));

			for (Entity_Count_t j = 0; j < entity_count; ++j)
			{
				const auto new_entity = Entity(storage.m_next_entity_ID++);
				storage.m_entity_to_archetype_ID.push_back(std::make_optional(std::make_pair(archetype_ID, archetype.m_next_instance_ID)));

				{// Add new_entity to the archetype. Similar to Archetype::push_back(Entity, ComponentTypes...)
					const BufferPosition index_start_pos = archetype.m_instance_size * archetype.m_next_instance_ID; // Position of the start of the entity instance in the archetype.

					for (const auto& component_layout : components)
					{
						const BufferPosition component_start_pos = index_start_pos + component_layout.offset;
						component_layout.type_info.Deserialise(&archetype.m_data[component_start_pos], p_in, p_version);
					}

					archetype.m_entities.push_back(new_entity);
					archetype.m_next_instance_ID++;
				}
			}
		}
		return storage;
	}
}