#pragma once

#include "Utility/Logger.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "Entity.hpp"
#include "Component.hpp"
#include "Meta.hpp"

namespace ECS
{
	constexpr bool Log_ECS_events = false;
	constexpr size_t Archetype_Start_Capacity = 32;

	using ArchetypeID         = size_t;
	using ArchetypeInstanceID = size_t; // Per ArchetypeID ID per component archetype instance.
	using BufferPosition      = size_t; // Used to index into archetype m_data.

	// Returns the next higher power of 2 after p_val
	inline size_t next_greater_power_of_2(const size_t& p_val)
	{ // Find the next power of 2 by shifting the bit to the left
		size_t result = 1;
		while (result <= p_val) result <<= 1;
		return result;
	}
	// Returns the multiple of p_multiple greater than p_min
	inline size_t next_multiple(const size_t& p_multiple, const size_t& p_min)
	{
		// If p_min is already a multiple of p_multiple, return p_min
		// Otherwise calculate the next multiple of p_multiple greater than or equal to p_min.

		if (p_min % p_multiple == 0)
			return p_min;
		else
			return ((p_min / p_multiple) + 1) * p_multiple;
	}

	// Describes the layout of a ComponentType in an Archetype instance.
	struct ComponentLayout
	{
		BufferPosition offset = 0;  // The number of bytes from the start of an Archetype instance to this Component
		ComponentData  type_info; // The ComponentData for this ComponentType.
	};

	// Returns the stride for a list of ComponentLayouts.
	inline size_t get_stride(const std::vector<ComponentLayout>& p_component_layouts)
	{
		size_t max_position = 0;
		size_t max_allign = 0;

		for (const auto& component : p_component_layouts)
		{
			max_position = std::max(max_position, component.offset + component.type_info.size);
			max_allign   = std::max(max_allign, component.type_info.align);
		}

		return next_multiple(max_allign, max_position);
	}

	// Returns the string representation of the memory layout for a list of ComponentLayouts.
	// Depends on p_component_layouts being ordered in ascending offset order.
	inline std::string to_string(const std::vector<ComponentLayout>& p_component_layouts)
	{
		const auto stride          = get_stride(p_component_layouts);
		const char padding_symbol  = '-';
		char running_char          = 'A';
		std::string component_list = "";
		component_list.reserve(p_component_layouts.size() * 3);
		std::string mem_layout     = "";
		mem_layout.reserve(stride);

		for (size_t i = 0; i < p_component_layouts.size(); i++)
		{
			const auto& component      = p_component_layouts[i];
			const auto component_label = running_char++;

			if (!component_list.empty()) component_list += ", ";
			component_list += std::format("\nID: {} ({}) size: {} align: {}", std::to_string(component.type_info.ID), component_label, component.type_info.size, component.type_info.align);

			const auto component_end_position = component.offset + component.type_info.size;
			const size_t padding_size         = i + 1 == p_component_layouts.size() ? stride - component_end_position : p_component_layouts[i + 1].offset - component_end_position;

			std::string comp_mem_layout = "";
			comp_mem_layout.reserve(component.type_info.size + padding_size);

			for (size_t j = 0; j < component.type_info.size; j++)
				comp_mem_layout += component_label;
			for (size_t j = 0; j < padding_size; j++)
				comp_mem_layout += padding_symbol;

			mem_layout += comp_mem_layout;
		}

		return std::format("{}:\n{} stride={}", component_list, mem_layout, stride);
	}

	// Generates a vector of ComponentLayouts from a paramater pack of ComponentTypes. Skips over Entity params.
	// This function sets out the order and allignment of the Components within the Archetype buffer.
	// The order of components is not guaranteed to remain the same as it appears in the bitset ID order.
	inline std::vector<ComponentLayout> get_components_layout(const ComponentBitset& p_component_bitset)
	{
		// There are a few assumptions we make setting the layout.
		// 1. alignof each component is always a power of 2 (guaranteed by standard - alignas(3) does not compile)
		// 2. We make no promise to store the types in the same order specified by parameter pack - we can pack more efficiently.
		// 3. Every ComponentType is at an offset position which is a multiple of its alignof.

		// Of all the ComponentTypes, this is the largest alignof value.
		size_t max_allignof = 0;
		// The most un-optimised buffer size deduced by summing the ComponentTypes sizes + padding in the order they appear in the pack.
		std::vector<ComponentLayout> component_layouts;
		component_layouts.reserve(p_component_bitset.count());
		for (size_t i = 0; i < p_component_bitset.size(); i++)
		{
			if (p_component_bitset[i])
			{
				const auto& info = Component::get_info(static_cast<ComponentID>(i));
				max_allignof = std::max(max_allignof, info.align);
				component_layouts.push_back({0, info});
			}
		}

		size_t worst_placement_size = 0;
		for (const auto& component : component_layouts)
			worst_placement_size += next_multiple(max_allignof, component.type_info.size);

		// TODO Sort by size (+ align if size is equal) descending
		std::sort(component_layouts.begin(), component_layouts.end(), [](const auto& a, const auto& b) -> bool { return a.type_info.size > b.type_info.size; });

		// Unused fragment of the buffer.
		struct empty_block
		{
			size_t start = 0;
			size_t size  = 0;
		};

		// Begin the placement algorithm with an empty block representing a completely empty buffer
		// worst_placement_size is neccessary since using size_t::max causes overflow problems below.
		std::vector<empty_block> empty_blocks = {{0, worst_placement_size}};

		// Go through the component_layouts and set their offset values.
		for (size_t i = 0; i < component_layouts.size(); i++)
		{
			for (size_t j = 0; j < empty_blocks.size(); j++)
			{
				if (empty_blocks[j].size >= component_layouts[i].type_info.size) // If the block is big enough for the type
				{
					const auto next_align_pos = next_multiple(component_layouts[i].type_info.align, empty_blocks[j].start);
					const auto block_end      = empty_blocks[j].start + empty_blocks[j].size;

					if (next_align_pos < block_end) // If the next alignment is before the end of the block
					{
						// If the remaining size of the block with alignment accounted for is large enough for this type
						const auto size_remaining = block_end - next_align_pos;
						if (size_remaining >= component_layouts[i].type_info.size)
						{ // The remaining size in the block is enough, we can fit this type into the empty_block
							component_layouts[i].offset = next_align_pos;

							{ // With the type offset assigned, the empty_block needs to be split/removed
								const auto type_end = component_layouts[i].offset + component_layouts[i].type_info.size;

								// Front block
								if (component_layouts[i].offset > empty_blocks[j].start)
									empty_blocks.push_back({empty_blocks[j].start, component_layouts[i].offset - empty_blocks[j].start});
								// Back block
								if (type_end < block_end)
									empty_blocks.push_back({type_end, block_end - type_end});
							}

							empty_blocks.erase(empty_blocks.begin() + j);
							break; // Iteration ends here, j is invalidated by the erase + we have set the type offset
						}
					}
				}
			}
			ASSERT(component_layouts[i].offset != 0 || i == 0, "Failed to set the position of ComponentID {} in the buffer.", component_layouts[i].type_info.ID);
		}

		std::sort(component_layouts.begin(), component_layouts.end(), [](const auto& a, const auto& b) -> bool { return a.offset < b.offset; });
		return component_layouts;
	}

	// Returns true if all of the ComponentTypes in the ComponentBitset are serialisable.
	inline bool is_serialisable(const ComponentBitset& p_component_bitset)
	{
		for (size_t i = 0; i < p_component_bitset.size(); i++)
		{
			if (p_component_bitset[i] && !Component::get_info(static_cast<ComponentID>(i)).is_serialisable)
				return false;
		}
		return true;
	}

	// A container of Entity objects and the components they own.
	// Every unique combination of components makes an Archetype which is a contiguous store of all the ComponentTypes.
	// Storage is interfaced using Entity as a key.
	class Storage
	{
		// Archetype is defined as a unique combination of ComponentTypes. It is a non-templated class allowing any combination of unique types to be stored in its m_data at runtime.
		// The ComponentTypes are retrievable using get_component and getComponentImpl as well as their 'Mutable' variants.
		// Every archetype stores its m_bitset for matching ComponentTypes.
		// mComponentLayout sets out how those ComponentTypes are laid out in an ArchetypeInstanceID.
		struct Archetype
		{
			ComponentBitset m_bitset;                  // The unique identifier for this archetype. Each bit corresponds to a ComponentType this archetype stores per ArchetypeInstanceID.
			std::vector<ComponentLayout> m_components; // How the ComponentTypes are laid out in each instance of ArchetypeInstanceID.
			bool m_is_serialisable;                    // If all of the ComponentTypes in this archetype are serialisable.
			std::vector<Entity> m_entities;            // Entity at every ArchetypeInstanceID. Should be indexed only using ArchetypeInstanceID.
			size_t m_instance_size;                    // Size in Bytes of each archetype instance. In other words, the stride between every ArchetypeInstanceID.
			ArchetypeInstanceID m_next_instance_ID;    // The ArchetypeInstanceID past the end of the m_data. Equivalant to size() in a vector.
			ArchetypeInstanceID m_capacity;            // The ArchetypeInstanceID count of how much memory is allocated in m_data for storage of components.
			std::byte* m_data;

			// Construct an Archetype from a template list of ComponentTypes.
			template<typename... ComponentTypes>
			Archetype(Meta::PackArgs<ComponentTypes...>) noexcept
				: m_bitset{Component::get_component_bitset<ComponentTypes...>()}
				, m_components{get_components_layout(m_bitset)}
				, m_is_serialisable{is_serialisable(m_bitset)}
				, m_entities{}
				, m_instance_size{get_stride(m_components)}
				, m_next_instance_ID{0}
				, m_capacity{Archetype_Start_Capacity}
				, m_data{(std::byte*)malloc(m_instance_size * m_capacity)}
			{
				if constexpr (Log_ECS_events) LOG("[ECS][Archetype] New Archetype created from components: {}", to_string(m_components));
			}

			// Construct an Archetype from a ComponentBitset.
			Archetype(const ComponentBitset& p_component_bitset) noexcept
				: m_bitset{p_component_bitset}
				, m_components{get_components_layout(m_bitset)}
				, m_is_serialisable{is_serialisable(m_bitset)}
				, m_entities{}
				, m_instance_size{get_stride(m_components)}
				, m_next_instance_ID{0}
				, m_capacity{Archetype_Start_Capacity}
				, m_data{(std::byte*)malloc(m_instance_size * m_capacity)}
			{
				if constexpr (Log_ECS_events) LOG("[ECS][Archetype] New Archetype created from components: {}", to_string(m_components));
			}

			~Archetype() noexcept
			{  // Call the destructor for all the components and free the heap memory.
				clear();
				free(m_data);

				if constexpr (Log_ECS_events) LOG("[ECS][Archetype] Destroyed at address {}", (void*)(this));
			}

			// Move-construct
			Archetype(Archetype&& p_other) noexcept
				: m_bitset{std::move(p_other.m_bitset)}
				, m_components{std::move(p_other.m_components)}
				, m_is_serialisable{std::move(p_other.m_is_serialisable)}
				, m_entities{std::move(p_other.m_entities)}
				, m_instance_size{std::move(p_other.m_instance_size)}
				, m_next_instance_ID{std::move(p_other.m_next_instance_ID)}
				, m_capacity{std::move(p_other.m_capacity)}
				, m_data{std::exchange(p_other.m_data, nullptr)}
			{
				if constexpr (Log_ECS_events) LOG("[ECS][Archetype] Move constructed {} from {}", (void*)(this), (void*)(&p_other));
			}
			// Move-assign
			Archetype& operator=(Archetype&& p_other) noexcept
			{
				if (this != &p_other)
				{
					if (m_data != nullptr)
					{
						clear();
						free(m_data);
					}

					m_bitset           = std::move(p_other.m_bitset);
					m_components       = std::move(p_other.m_components);
					m_is_serialisable  = std::move(p_other.m_is_serialisable);
					m_entities         = std::move(p_other.m_entities);
					m_instance_size    = std::move(p_other.m_instance_size);
					m_next_instance_ID = std::move(p_other.m_next_instance_ID);
					m_capacity         = std::move(p_other.m_capacity);
					m_data             = std::exchange(p_other.m_data, nullptr);
				}

				if constexpr (Log_ECS_events) LOG("[ECS][Archetype] Move assigning {} from {}", (void*)(this), (void*)(&p_other));
				return *this;
			}
			// Copy-construct
			Archetype(const Archetype& p_other)
				: m_bitset{p_other.m_bitset}
				, m_components{p_other.m_components}
				, m_is_serialisable{p_other.m_is_serialisable}
				, m_entities{p_other.m_entities}
				, m_instance_size{p_other.m_instance_size}
				, m_next_instance_ID{p_other.m_next_instance_ID}
				, m_capacity{p_other.m_capacity}
				, m_data{(std::byte*)malloc(m_instance_size * m_capacity)}
			{
				// Copy construct all the components from p_other into this.
				if (p_other.m_data != nullptr)
				{
					for (ArchetypeInstanceID instance = 0; instance < m_next_instance_ID; instance++)
					{
						const auto instance_start = m_instance_size * instance;

						for (const auto& comp : m_components)
						{
							const auto address_offset = instance_start + comp.offset;
							comp.type_info.CopyConstruct(&m_data[address_offset], &p_other.m_data[address_offset]);
						}
					}
				}

				if constexpr (Log_ECS_events) LOG("[ECS][Archetype] Copy constructed {} from {}", (void*)(this), (void*)(&p_other));
			}
			// Copy-assign
			Archetype& operator=(const Archetype& p_other)
			{
				if (this != &p_other)
				{
					if (m_data != nullptr)
					{
						clear();
						free(m_data);
					}

					m_bitset           = p_other.m_bitset;
					m_components       = p_other.m_components;
					m_is_serialisable  = p_other.m_is_serialisable;
					m_entities         = p_other.m_entities;
					m_instance_size    = p_other.m_instance_size;
					m_next_instance_ID = p_other.m_next_instance_ID;
					m_capacity         = p_other.m_capacity;
					m_data             = (std::byte*)malloc(m_instance_size * m_capacity);

					// Copy construct all the components from p_other into this.
					if (p_other.m_data != nullptr)
					{
						for (ArchetypeInstanceID instance = 0; instance < m_next_instance_ID; instance++)
						{
							const auto instance_start = m_instance_size * instance;

							for (const auto& comp : m_components)
							{
								const auto address_offset = instance_start + comp.offset;
								comp.type_info.CopyConstruct(&m_data[address_offset], &p_other.m_data[address_offset]);
							}
						}
					}
				}

				if constexpr (Log_ECS_events) LOG("[ECS][Archetype] Copy assigned {} from {}", (void*)(this), (void*)(&p_other));
				return *this;
			}

			// Search the m_components vector for the p_component_ID and return its ComponentLayout.
			// Non-template version (when we know the ComponentID but not the Type).
			const ComponentLayout& get_component_layout(ComponentID p_component_ID) const
			{
				// Linear search for the ComponentLayout with the matching ComponentID.
				auto it = std::find_if(m_components.begin(), m_components.end(), [&p_component_ID](const auto& p_component_layout)
					{ return p_component_layout.type_info.ID == p_component_ID; });

				ASSERT_THROW(it != m_components.end(), "Requested a ComponentLayout for a ComponentType not present in this archetype.");
				return *it;
			}

			// Search the m_components vector for the ComponentType and return its ComponentLayout.
			template <typename ComponentType>
			const ComponentLayout& get_component_layout() const
			{
				return get_component_layout(Component::get_ID<ComponentType>());
			}

			// Get the byte offset of ComponentType from the start of any ArchetypeInstanceID.
			template <typename ComponentType>
			BufferPosition get_component_offset() const
			{
				return get_component_layout<ComponentType>().offset;
			}

			// Get the byte position of ComponentType at ArchetypeInstanceID.
			template <typename ComponentType>
			BufferPosition get_component_position(const ArchetypeInstanceID& p_instance_index) const
			{
				const auto indexStartPosition = m_instance_size * p_instance_index; // Position of the start of the instance at p_instance_index.
				return indexStartPosition + get_component_offset<ComponentType>();
			}

			// Returns a const pointer to the ComponentType at p_instance_index.
			// The position of this component is found using a linear search of mComponentLayouts. If the BufferPosition is known use reinterpret_cast directly.
			template <typename ComponentType>
			const std::decay_t<ComponentType>* get_component(const ArchetypeInstanceID& p_instance_index) const
			{
				const auto component_position = get_component_position<ComponentType>(p_instance_index);
				return reinterpret_cast<const std::decay_t<ComponentType>*>(&m_data[component_position]);
			}
			// Returns a pointer to the ComponentType at p_instance_index.
			// The position of this component is found using a linear search of mComponentLayouts. If the BufferPosition is known use reinterpret_cast directly.
			template <typename ComponentType>
			std::decay_t<ComponentType>* get_component(const ArchetypeInstanceID& p_instance_index)
			{
				const auto component_position = get_component_position<ComponentType>(p_instance_index);
				return reinterpret_cast<std::decay_t<ComponentType>*>(&m_data[component_position]);
			}

			// Inserts the components from the provided paramater pack ComponentTypes into the Archetype at the end.
			// If the archetype is full, more memory is reserved increasing the Archetype capacity.
			template <typename... ComponentTypes>
			void push_back(const Entity& p_entity, ComponentTypes&&... p_component_values)
			{
				static_assert(Meta::is_unique<ComponentTypes...>, "Non unique component types! Archetype can only push back a set of unique ComponentTypes");

				if (m_next_instance_ID + 1 > m_capacity)
					reserve(next_greater_power_of_2(m_capacity));

				// Each `ComponentType` in the parameter pack is placement-new constructed into m_data preserving the value category of the parameter.
				auto construct_func = [&](auto&& p_component)
				{
					using ComponentType = std::decay_t<decltype(p_component)>;

					const auto component_start_position = get_component_position<ComponentType>(m_next_instance_ID);
					new (&m_data[component_start_position]) ComponentType(std::forward<decltype(p_component)>(p_component));
				};
				(construct_func(std::forward<ComponentTypes>(p_component_values)), ...); // Unfold construct_func over the ComponentTypes

				m_entities.push_back(p_entity);
				m_next_instance_ID++;
			}

			// Remove the instance of the archetype at p_erase_index.
			// Updates Archetype::m_entities container and Storage::m_entity_to_archetype_ID according to placement changes caused by erase. (Non-end erase uses swap and pop idiom).
			void erase(const ArchetypeInstanceID& p_erase_index, const Entity& p_entity, std::vector<std::optional<std::pair<ArchetypeID, ArchetypeInstanceID>>>& p_entity_to_archetype_ID)
			{
				if (p_erase_index >= m_next_instance_ID) throw std::out_of_range("Index out of range");

				const auto last_instance_start_position  = m_instance_size * (m_next_instance_ID - 1);  // Position of the start of the last instance.

				if (p_erase_index == m_next_instance_ID - 1)
				{ // If erasing off the end, call the destructors for all the components at the end index
					for (size_t comp = 0; comp < m_components.size(); comp++)
					{
						const auto last_instance_comp_start_position = last_instance_start_position + m_components[comp].offset;
						const auto last_instance_comp_address = &m_data[last_instance_comp_start_position];
						m_components[comp].type_info.Destruct(last_instance_comp_address);
					}
				}
				else
				{
					// Erasing an index not on the end of the Archetype
					// Move-assign the end components into the p_erase_index then call the destructor on all the end elements.

					const auto erase_instance_start_position = m_instance_size * p_erase_index; // Position of the start of the instance at p_erase_index.

					for (size_t comp = 0; comp < m_components.size(); comp++)
					{
						const auto last_instance_comp_start_position = last_instance_start_position + m_components[comp].offset;
						const auto last_instance_comp_address = &m_data[last_instance_comp_start_position];

						const auto erase_instance_comp_start_position = erase_instance_start_position + m_components[comp].offset;
						const auto erase_instance_comp_address = &m_data[erase_instance_comp_start_position];

						m_components[comp].type_info.MoveAssign(erase_instance_comp_address, last_instance_comp_address);
						m_components[comp].type_info.Destruct(last_instance_comp_address);
					}

					// Move the end_entity into the erased index and update the p_entity_to_archetype_ID bookeeping.
					auto end_entity = m_entities[m_entities.size() - 1];
					m_entities[p_erase_index] = end_entity;
					p_entity_to_archetype_ID[end_entity].value().second = p_erase_index;
				}

				m_entities.pop_back();
				m_next_instance_ID--;
				p_entity_to_archetype_ID[p_entity] = std::nullopt;
			}

			// Allocate the memory required for p_new_capacity archetype instances. The m_size of the archetype is unchanged.
			void reserve(const size_t& p_new_capacity)
			{
				if (p_new_capacity <= m_capacity)
					return;

				m_capacity = p_new_capacity;
				std::byte* new_data = (std::byte*)malloc(m_instance_size * m_capacity);

				// Placement-new move-construct the objects from this into the auxillary store.
				// Then call the destructor on the old instances that were moved.
				for (size_t i = 0; i < m_next_instance_ID; i++)
				{
					auto instance_start = m_instance_size * i;

					for (size_t comp = 0; comp < m_components.size(); comp++)
					{
						auto comp_data_position = instance_start + m_components[comp].offset;

						m_components[comp].type_info.MoveConstruct(&new_data[comp_data_position], &m_data[comp_data_position]);
						m_components[comp].type_info.Destruct(&m_data[comp_data_position]);
					}
				}

				free(m_data);
				m_data = new_data;
			}

			// Destroy all the components in all instances of this archetype.
			// Size is 0 after clear.
			void clear()
			{
				for (ArchetypeInstanceID instance = 0; instance < m_next_instance_ID; instance++)
				{
					const auto instance_start = m_instance_size * instance;

					for (auto& comp : m_components)
					{
						const auto comp_address = &m_data[instance_start + comp.offset];
						comp.type_info.Destruct(comp_address);
					}
				}

				m_next_instance_ID = 0;
			}
		}; // class Archetype

		EntityID m_next_entity_ID = 0;
		std::vector<Archetype> m_archetypes;
		// Maps EntityID to a position pair [ index in m_archetypes, ArchetypeInstanceID in archetype ].
		// Nullopt here means the entity was deleted. No gaps are created on delete so ID's are never reused.
		std::vector<std::optional<std::pair<ArchetypeID, ArchetypeInstanceID>>> m_entity_to_archetype_ID;

		template <typename... FunctionArgs>
		struct FunctionHelper;
		template <typename... FunctionArgs>
		struct FunctionHelper<Meta::PackArgs<FunctionArgs...>>
		{
			static_assert(Meta::is_unique<FunctionArgs...>, "Cannot construct a FunctionHelper from a list of types with duplicates. Are you calling foreach with repeating parameters?");
			static_assert(sizeof...(FunctionArgs) > 0, "Cannot construct a FunctionHelper with 0 types, are you calling foreach with 0 params?");

			static ComponentBitset get_bitset()
			{
				return ECS::Component::get_component_bitset<FunctionArgs...>();
			}
			// Does this function take only one parameter of type Entity.
			constexpr static bool is_entity_function()
			{
				if constexpr (sizeof...(FunctionArgs) == 1 && std::is_same_v<Entity, std::decay_t<typename Meta::GetNth<0, FunctionArgs...>::Type>>)
					return true;
				else
					return false;
			}
		};

		template <typename... FunctionArgs>
		struct ApplyFunction;
		template <typename Func, typename... FunctionArgs>
		struct ApplyFunction<Func, Meta::PackArgs<FunctionArgs...>>
		{
			static void apply_to_archetype(const Func& p_function, Archetype& p_archetype)
			{
				const auto index_sequence = std::index_sequence_for<FunctionArgs...>{};
				const auto offsets = getOffsets(p_archetype, index_sequence);
				impl(p_function, p_archetype, offsets, index_sequence);
			}

		private:
			// Given a p_function and p_archetype, calls p_function on every ArchetypeInstanceID supplying the ComponentTypes as arguments.
			// p_archetype_offsets: The mapping of p_function arguments to their offsets per ArchetypeInstanceID.
			// index_sequence:    Provides a mechanism to execute a fold expression to retrieve all the arguments from the Archetype.
			template <std::size_t... Is>
			static void impl(const Func& p_function, Archetype& p_archetype, const std::array<BufferPosition, sizeof...(FunctionArgs)>& p_archetype_offsets, const std::index_sequence<Is...>&)
			{ // If we have reached this point we can guarantee p_archetype contains all the components in FunctionArgs.
				for (ArchetypeInstanceID i = 0; i < p_archetype.m_next_instance_ID; i++)
					p_function(*get_from_archetype<FunctionArgs>(p_archetype, i, p_archetype_offsets[Is])...);
			}

			// Get a ComponentType* from p_archetype at p_index with p_offset.
			template <typename ComponentType>
			static std::decay_t<ComponentType>* get_from_archetype(Archetype& p_archetype, const ArchetypeInstanceID& p_index, const size_t& p_offset)
			{
				if constexpr (std::is_same_v<Entity, std::decay_t<ComponentType>>)
					return &p_archetype.m_entities[p_index];
				else
					return reinterpret_cast<std::decay_t<ComponentType>*>(&p_archetype.m_data[(p_archetype.m_instance_size * p_index) + p_offset]);
			}

			// Assign the Byte offset of the ComponentType in p_archetype into p_offsets at p_index. Skips over Entity's encountered.
			template <typename ComponentType, std::size_t... Is>
			static void set_offset(std::array<BufferPosition, sizeof...(FunctionArgs)>& p_offsets, const size_t& p_index, const Archetype& p_archetype)
			{
				if constexpr (!std::is_same_v<Entity, std::decay_t<ComponentType>>) // Ignore any Entity params supplied.
					p_offsets[p_index] = p_archetype.get_component_layout<ComponentType>().offset;
			}

			// Construct an array of corresponding to the offset of each FunctionArgs into the archetype.
			// Entity types encountered will not be set but the index in the returned array will exist.
			template <std::size_t... Is>
			static std::array<BufferPosition, sizeof...(FunctionArgs)> getOffsets(const Archetype& p_archetype, const std::index_sequence<Is...>&)
			{
				std::array<BufferPosition, sizeof...(FunctionArgs)> offsets;
				(set_offset<FunctionArgs>(offsets, Is, p_archetype), ...);
				return offsets;
			}
		};

		// Find the ArchetypeID with the exact matching componentBitset.
		// Every Archetype has a unique bitset so we can guarantee only one exists.
		// Returns nullopt if this archtype hasnt been added to m_archetypes yet.
		std::optional<ArchetypeID> get_matching_archetype(const ComponentBitset& p_component_bitset)
		{
			for (ArchetypeID i = 0; i < m_archetypes.size(); i++)
			{
				if (p_component_bitset == m_archetypes[i].m_bitset)
					return i;
			}

			return std::nullopt;
		};
		// Find the ArchetypeIDs of any Archetypes with the exact matching componentBitset or containing it.
		// Returns an empty vec if there are none.
		std::vector<ArchetypeID> get_matching_or_contained_archetypes(const ComponentBitset& p_component_bitset)
		{
			std::vector<ArchetypeID> return_vec;

			for (ArchetypeID i = 0; i < m_archetypes.size(); i++)
			{
				if (p_component_bitset == m_archetypes[i].m_bitset || ((p_component_bitset & m_archetypes[i].m_bitset) == p_component_bitset))
					return_vec.push_back(i);
			}

			return return_vec;
		};

	public:
		// Creates an Entity out of the ComponentTypes.
		// The ComponentTypes must all be unique, only one of each ComponentType can be owned by an Entity.
		// The ComponentTypes can be retrieved individually using get_component or as a combination using foreach.
		template <typename... ComponentTypes>
		Entity add_entity(ComponentTypes&&... p_components)
		{
			static_assert(Meta::is_unique<ComponentTypes...>, "add_entity non-unique list of components given.");

			const ComponentBitset bitset = Component::get_component_bitset<ComponentTypes...>();
			auto archetype_ID = get_matching_archetype(bitset);

			if (!archetype_ID)
			{// No matching archetype was found we add a new one for this ComponentBitset.
				m_archetypes.push_back(Archetype(Meta::PackArgs<ComponentTypes...>()));
				archetype_ID = m_archetypes.size() - 1;
			}

			const auto new_entity = Entity(m_next_entity_ID++);
			auto& archetype = m_archetypes[archetype_ID.value()];
			archetype.push_back(new_entity, std::forward<ComponentTypes>(p_components)...);
			m_entity_to_archetype_ID.push_back(std::make_optional(std::make_pair(archetype_ID.value(), archetype.m_next_instance_ID - 1)));

			return new_entity;
		}
		// Removes p_entity from storage.
		// The associated Entity is then on invalid for invoking other Storage funcrions on.
		void delete_entity(const Entity& p_entity)
		{
			const auto& [archetype, erase_index] = *m_entity_to_archetype_ID[p_entity.ID];
			m_archetypes[archetype].erase(erase_index, p_entity, m_entity_to_archetype_ID);
		}

		// Calls Func on every Entity which owns all of the components arguments of p_function.
		// p_function can have any number of ComponentTypes but will only be called if the Entity owns all of the components or more.
		// An optional Entity param in function will be supplied the Entity which owns the ComponentTypes on each call of p_function.
		template <typename Func>
		void foreach(const Func& p_function)
		{
			using FunctionParameterPack = typename Meta::GetFunctionInformation<Func>::GetParameterPack;

			if constexpr (FunctionHelper<FunctionParameterPack>::is_entity_function())
			{
				for (EntityID i = 0; i < m_entity_to_archetype_ID.size(); i++)
				{
					if (m_entity_to_archetype_ID[i].has_value())
					{
						auto ent = Entity(i);
						p_function(ent);
					}
				}
			}
			else
			{
				const auto function_bitset = FunctionHelper<FunctionParameterPack>::get_bitset();
				const auto archetype_IDs   = get_matching_or_contained_archetypes(function_bitset);

				if (!archetype_IDs.empty())
				{
					for (auto& archetype_ID : archetype_IDs)
					{
						if (m_archetypes[archetype_ID].m_next_instance_ID > 0)
						{
							ApplyFunction<Func, FunctionParameterPack>::apply_to_archetype(p_function, m_archetypes[archetype_ID]);
						}
					}
				}
			}
		}

		// Get a reference to component of ComponentType belonging to Entity.
		// If Entity doesn't own one, an exception will be thrown. Owned ComponentTypes can be queried using has_components.
		//@param p_entity The Entity to get the component from.
		//@return A const reference to the component.
		template <typename ComponentType>
		[[nodiscard]] const std::decay_t<ComponentType>& get_component(const Entity& p_entity) const
		{
			const auto [archetype, index] = *m_entity_to_archetype_ID[p_entity.ID];
			return *m_archetypes[archetype].get_component<ComponentType>(index);
		}

		// Get a reference to component of ComponentType belonging to Entity.
		// If Entity doesn't own one, an exception will be thrown. Owned ComponentTypes can be queried using has_components.
		//@param p_entity The Entity to get the component from.
		//@return A reference to the component.
		template <typename ComponentType>
		[[nodiscard]] std::decay_t<ComponentType>& get_component(const Entity& p_entity)
		{
			const auto [archetype, index] = *m_entity_to_archetype_ID[p_entity.ID];
			return *m_archetypes[archetype].get_component<ComponentType>(index);
		}

		// Add the p_component to p_entity. If p_entity already owns this ComponentType, do nothing.
		template <typename ComponentType>
		void add_component(const Entity& p_entity, ComponentType&& p_component)
		{
			const auto& [from_archetype_ID, from_archetype_index] = *m_entity_to_archetype_ID[p_entity.ID];
			const auto add_component_ID = Component::get_ID<ComponentType>();

			if (m_archetypes[from_archetype_ID].m_bitset[add_component_ID]) // p_entity already own this ComponentType, do nothing.
				return;

			// The bitset of p_entity with ComponentType added. This is the bitset for the archetype the current p_entity Components are being moved into.
			auto bitset = m_archetypes[from_archetype_ID].m_bitset;
			bitset[add_component_ID] = true;
			auto to_archetype_ID = get_matching_archetype(bitset);

			// If there is no archetype matching p_entity ComponentTypes after removing ComponentType, create a new one and set to_archetype to it.
			if (!to_archetype_ID.has_value())
			{
				m_archetypes.push_back(Archetype(bitset));
				to_archetype_ID = m_archetypes.size() - 1;
			}

			// We now know from_archetype and to_archetype this Entity will be traversing.
			// Move-construct the p_entity components from_archetype into to_archetype and destruct the from_archetype components.
			// Updates Archetype::m_entities containers and Storage::m_entity_to_archetype_ID according to placement changes caused by inheriting p_entity and required erase.
			{
				auto& from_archetype = m_archetypes[from_archetype_ID];
				auto& to_archetype   = m_archetypes[to_archetype_ID.value()];

				if (to_archetype.m_next_instance_ID >= to_archetype.m_capacity)
					to_archetype.reserve(next_greater_power_of_2(to_archetype.m_capacity));

				// Move construct all the components into to_archetype from from_archetype.
				// Then call erase on the index/entity in from_archetype.
				{
					const auto from_instance_start = from_archetype.m_instance_size * from_archetype_index;
					const auto to_end_instance_start = to_archetype.m_instance_size * to_archetype.m_next_instance_ID;

					for (auto& comp : from_archetype.m_components)
					{
						const auto from_comp_address = &from_archetype.m_data[from_instance_start + comp.offset];
						const auto to_comp_address = &to_archetype.m_data[to_end_instance_start + to_archetype.get_component_layout(comp.type_info.ID).offset];
						comp.type_info.MoveConstruct(to_comp_address, from_comp_address);
						// from_archetype.erase handles calling the destructors.
					}

					// Placement-new construct p_component into m_data preserving the value category.
					const auto add_component_start_position = to_end_instance_start + to_archetype.get_component_layout(add_component_ID).offset;
					new (&to_archetype.m_data[add_component_start_position]) std::decay_t<ComponentType>(std::forward<decltype(p_component)>(p_component));

					// Update m_entities and m_entity_to_archetype_ID.
					from_archetype.erase(from_archetype_index, p_entity, m_entity_to_archetype_ID);
					to_archetype.m_entities.push_back(p_entity);
					to_archetype.m_next_instance_ID++;
					m_entity_to_archetype_ID[p_entity] = std::make_optional(std::make_pair(to_archetype_ID.value(), to_archetype.m_next_instance_ID - 1));
				}
			}
		}

		// Delete the ComponentType belonging to p_entity.
		template <typename ComponentType>
		void delete_component(const Entity& p_entity)
		{
			if (!m_entity_to_archetype_ID[p_entity.ID].has_value()) // p_entity has been deleted
				return;

			const auto& [from_archetype_ID, from_archetype_index] = *m_entity_to_archetype_ID[p_entity.ID];
			const auto delete_component_ID = Component::get_ID<ComponentType>();
			if (!m_archetypes[from_archetype_ID].m_bitset[delete_component_ID]) // p_entity doesnt own this ComponentType already, do nothing.
				return;
			else if (m_archetypes[from_archetype_ID].m_components.size() == 1) // from_archetype is a single component delete_component == erase.
			{
				m_archetypes[from_archetype_ID].erase(from_archetype_index, p_entity, m_entity_to_archetype_ID);
				return;
			}
			// The bitset of p_entity with ComponentType removed. This is the bitset for the archetype the remaining Components are being moved into.
			auto bitset = m_archetypes[from_archetype_ID].m_bitset;
			bitset[delete_component_ID] = false;
			auto to_archetype_ID = get_matching_archetype(bitset);

			// If there is no archetype matching p_entity ComponentTypes after removing ComponentType, create a new one and set to_archetype to it.
			if (!to_archetype_ID.has_value())
			{
				m_archetypes.push_back(Archetype(bitset));
				to_archetype_ID = m_archetypes.size() - 1;
			}

			// We now know from_archetype and to_archetype this Entity will be traversing.
			// Move-construct the p_entity components from_archetype that fit into to_archetype and destruct the from_archetype components.
			// Updates Archetype::m_entities containers and Storage::m_entity_to_archetype_ID according to placement changes caused by inheriting p_entity and required erase.
			{
				auto& from_archetype = m_archetypes[from_archetype_ID];
				auto& to_archetype   = m_archetypes[to_archetype_ID.value()];

				if (to_archetype.m_next_instance_ID >= to_archetype.m_capacity)
					to_archetype.reserve(next_greater_power_of_2(to_archetype.m_capacity));

				// Move-construct all the components into to_archetype end from from_archetype.
				// Then call erase on the index/entity in from_archetype.
				{
					const auto from_instance_start   = from_archetype.m_instance_size * from_archetype_index;
					const auto to_end_instance_start = to_archetype.m_instance_size * to_archetype.m_next_instance_ID;

					for (auto& comp : from_archetype.m_components)
					{
						const auto from_comp_address = &from_archetype.m_data[from_instance_start + comp.offset];

						if (comp.type_info.ID != delete_component_ID)
						{
							const auto to_comp_address = &to_archetype.m_data[to_end_instance_start + to_archetype.get_component_layout(comp.type_info.ID).offset];
							comp.type_info.MoveConstruct(to_comp_address, from_comp_address);
							// from_archetype.erase handles calling the destructors.
						}
					}

					// Update m_entities and m_entity_to_archetype_ID.
					from_archetype.erase(from_archetype_index, p_entity, m_entity_to_archetype_ID);
					to_archetype.m_entities.push_back(p_entity);
					to_archetype.m_next_instance_ID++;
					m_entity_to_archetype_ID[p_entity] = std::make_optional(std::make_pair(to_archetype_ID.value(), to_archetype.m_next_instance_ID - 1));
				}
			}
		}

		// Check if Entity has been assigned all of the ComponentTypes queried. (Can be called with a single ComponentType)
		template <typename... ComponentTypes>
		[[nodiscard]] bool has_components(const Entity& p_entity) const
		{
			static_assert(sizeof...(ComponentTypes) != 0, "Cannot query has_components with 0 types.");

			if (!m_entity_to_archetype_ID[p_entity.ID].has_value()) // p_entity has been deleted
				return false;

			if constexpr (sizeof...(ComponentTypes) > 1)
			{// Grab the archetype bitset the entity belongs to and check if the ComponentTypes bitset matches or is a subset of it.
				const auto requested_bitset = Component::get_component_bitset<ComponentTypes...>();
				const auto [archetype, index] = *m_entity_to_archetype_ID[p_entity.ID];
				const auto entityBitset = m_archetypes[archetype].m_bitset;
				return (requested_bitset == entityBitset || ((requested_bitset & entityBitset) == requested_bitset));
			}
			else
			{// If we only have one requested ComponentType, we can skip the ComponentTypes bitset construction and test just the corresponding bit.
				typedef typename Meta::GetNth<0, ComponentTypes...>::Type ComponentType;
				const auto [archetype, index] = *m_entity_to_archetype_ID[p_entity.ID];
				return m_archetypes[archetype].m_bitset.test(Component::get_ID<ComponentType>());
			}
		}

		// Return the number of components of ComponentType in the storage.
		template <typename... ComponentTypes>
		[[nodiscard]] size_t count_components() const
		{
			static_assert(sizeof...(ComponentTypes) != 0, "Cannot query count_components with 0 types.");

			// Grab the archetype bitset the entity belongs to and check if the ComponentTypes bitset matches or is a subset of it.
			const auto requested_bitset = Component::get_component_bitset<ComponentTypes...>();
			size_t count = 0;

			for (const auto& archetype : m_archetypes)
			{
				if (requested_bitset == archetype.m_bitset || ((requested_bitset & archetype.m_bitset) == requested_bitset))
					count += archetype.m_next_instance_ID;
			}

			return count;
		}

		[[nodiscard]] size_t count_entities() const
		{
			size_t count = 0;

			for (const auto& archetype : m_archetypes)
				count += archetype.m_next_instance_ID;

			return count;
		}

		// Write the state of the storage to p_file stream.
		static void serialise(std::ostream& p_out, uint16_t p_version, const Storage& p_storage);
		// Construct a Storage from the state in p_file stream.
		static Storage deserialise(std::istream& p_in, uint16_t p_version);
	};
} // namespace ECS