#pragma once

#include "Logger.hpp"

#include <algorithm>
#include <array>
#include <bitset>
#include <iostream>
#include <optional>
#include <typeindex>
#include <typeinfo>
#include <vector>

#include "Meta.hpp"

namespace ECS
{
    constexpr size_t Max_Component_Count = 32;
    constexpr size_t Archetype_Start_Capacity = 32;

    using EntityID            = size_t;
    using ArchetypeID         = size_t;
    using ArchetypeInstanceID = size_t; // Per ArchetypeID ID per component archetype instance.
    using BufferPosition      = size_t; // Used to index into archetype m_data.
    using ComponentID         = size_t; // Unique identifier for any type passed into ECSStorage.
    using ComponentBitset     = std::bitset<Max_Component_Count>;

    class Entity
    {
    public:
        EntityID ID;

        operator EntityID () const { return ID; } // Implicitly convert an Entity to an EntityID.
    };
    // MemberFuncs wraps pointers to special member functions of classes.
    // These are neccessary as they need to be accessed after type erasure within the Archetype after construction e.g. erase(Index), reserve(Capacity).
    // By virtue of type erasure, there is no type safety or runtime check to assert the pointers given to the special functions correspond to the type they were constructed with.
    // This is a modified version of https://herbsutter.com/2016/09/25/to-store-a-destructor/
    struct MemberFuncs
    {
        MemberFuncs()
            : Destruct([](void*) {})
            , MoveAssign([](void* p_destination_address, void* p_source_address) {})
            , MoveConstruct([](void* p_destination_address, void* p_source_address) {})
        {}

        template <typename T>
        MemberFuncs(Meta::PackArg<T>)
            : Destruct{
                  [](void* p_address)
                  {
                      using Type = std::decay_t<T>;
                      static_cast<Type*>(p_address)->~Type();
                  }}
            , MoveAssign{[](void* p_destination_address, void* p_source_address)
                         {
                             using Type                                 = std::decay_t<T>;
                             *static_cast<Type*>(p_destination_address) = std::move(*static_cast<Type*>(p_source_address));
                         }}
            , MoveConstruct{[](void* p_destination_address, void* p_source_address)
                            {
                                using Type = std::decay_t<T>;
                                new (p_destination_address) Type(std::move(*static_cast<Type*>(p_source_address)));
                            }}
        {}

        // Call the destructor of the object at p_address_to_destroy.
        void (*Destruct)(void* p_address_to_destroy);
        // move-assign the object pointed to by p_source_address into the memory pointed to by p_destination_address.
        void (*MoveAssign)(void* p_destination_address, void* p_source_address);
        // placement-new move-construct the object pointed to by p_source_address into the memory pointed to by p_destination_address.
        void (*MoveConstruct)(void* p_destination_address, void* p_source_address);
    };

    struct ComponentInfo
    {
        ComponentID ID    = 0;
        size_t size       = 0;
        size_t align      = 0;
        MemberFuncs funcs = {};
    };

    struct ComponentLayout
    {
        BufferPosition offset = 0; // The number of bytes from the start of an Archetype instance to this Component
        ComponentInfo info    = {};
    };

    // ComponentHelper stores an array of ComponentInfos retrievable by ComponentType or ComponentID.
    // When Storage comes across a request to addEntity all the ComponentTypes are processed into the array so their data can be retrieved later.
    // Archetype uses type-erasure storing just the ComponentIDs which it can later use to retrieve ComponentInfo without the Type using get(ComponentID).
    // Calling a set<ComponentType>() before get<ComponentType>() is required. This is preferred to setting at every get to maximise speed of get. In the context of ECS, we know a ComponentType needs potential setting only when constructing an Archetype.
    class ComponentHelper
    {
       static inline std::array<std::optional<ComponentInfo>, Max_Component_Count> Infos = {};

       static inline ComponentID counter = 0;
       template <typename ComponentType>
       static inline ComponentID perComponentTypeID = counter++;

    public:
        template <typename ComponentType>
        static ComponentID get_ID()
        {
            using DecayedComponentType = std::decay_t<ComponentType>;
            return perComponentTypeID<DecayedComponentType>;
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
            (setComponentBit.operator()<ComponentTypes>(), ...);

            return componentBitset;
        }

        template <typename ComponentType>
        static inline void set_info()
        {
            auto ID = get_ID<ComponentType>();
            if (!Infos[ID].has_value())
            {
                using DecayedComponentType = std::decay_t<ComponentType>;
                Infos[ID]                  = std::make_optional<ComponentInfo>(ID, sizeof(DecayedComponentType), alignof(DecayedComponentType), Meta::PackArg<DecayedComponentType>());
                LOG("ComponentInfo set for {} ({}): ID: {}, size: {}, alignment: {}", typeid(ComponentType).name(), typeid(DecayedComponentType).name(), Infos[ID]->ID, Infos[ID]->size, Infos[ID]->align)
            }
        }
        template <typename... ComponentTypes>
        static inline void set_infos()
        {
            (set_info<ComponentTypes>(), ...);
        }

        template <typename ComponentType>
        static inline ComponentInfo get_info()
        {
            ASSERT(Infos[get_ID<ComponentType>()].has_value(), "Info for ComponentID {} is not set. Did you forget to call set_info for this ComponentType.", get_ID<ComponentType>());
            return Infos[get_ID<ComponentType>()].value();
        }
        static inline ComponentInfo get_info(const ComponentID& p_ID)
        {
            ASSERT(Infos[p_ID].has_value(), "Info for ComponentID {} is not set. Did you forget to call set_info for this ComponentType.", p_ID);
            return Infos[p_ID].value();
        }
    };

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

    // Returns the stride for a list of ComponentLayouts.
    inline size_t get_stride(const std::vector<ComponentLayout>& p_component_layouts)
    {
        size_t max_position = 0;
        size_t max_allign = 0;

        for (const auto& component : p_component_layouts)
        {
            max_position = std::max(max_position, component.offset + component.info.size);
            max_allign = std::max(max_allign, component.info.align);
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
            component_list += std::format("\nID: {} ({}) size: {} align: {}", std::to_string(component.info.ID), component_label, component.info.size, component.info.align);

            const auto component_end_position = component.offset + component.info.size;
            const size_t padding_size         = i + 1 == p_component_layouts.size() ? stride - component_end_position : p_component_layouts[i + 1].offset - component_end_position;

            std::string comp_mem_layout = "";
            comp_mem_layout.reserve(component.info.size + padding_size);

            for (size_t i = 0; i < component.info.size; i++)
                comp_mem_layout += component_label;
            for (size_t i = 0; i < padding_size; i++)
                comp_mem_layout += padding_symbol;

            mem_layout += comp_mem_layout;
        }

        return std::format("{}:\n{} stride={}", component_list, mem_layout, stride);
    }

    // Generates a vector of ComponentLayouts from a paramater pack of ComponentTypes. Skips over Entity params.
    // This function sets out the order and allignment of the Components within the Archetype buffer.
    // The order of components is not guaranteed to remain the same.
    template <typename... ComponentTypes>
    inline std::vector<ComponentLayout> get_components_layout()
    {
        // There are a few assumptions we make setting the layout.
        // 1. alignof each component is always a power of 2 (guaranteed by standard alignas(3) does not compile)
        // 2. We make no promise to store the types in the same order specified by parameter pack - we can pack more efficiently.
        // 3. Each ComponentType is at an offset position which is a multiple of its alignof.

        // Of all the ComponentTypes, this is the largest alignof value.
        constexpr auto max_allignof = Meta::get_max_alignof<ComponentTypes...>();
        // The most un-optimised buffer size deduced by summing the ComponentTypes sizes + padding in the order they appear in the pack.
        size_t worst_placement_size = 0;

        std::vector<ComponentLayout> component_layouts;
        component_layouts.reserve(sizeof...(ComponentTypes));

        auto set_component_info = [&component_layouts, &worst_placement_size]<typename ComponentType>()
        {
            if constexpr (!std::is_same_v<Entity, std::decay_t<ComponentType>>) // Ignore any Entity params supplied.
            {
                component_layouts.push_back({0, ComponentHelper::get_info<ComponentType>()});
                worst_placement_size += next_multiple(max_allignof, sizeof(ComponentType));
            }
        };
        (set_component_info.operator()<ComponentTypes>(), ...);

        // TODO Sort by size (+ align if size is equal) descending
        std::sort(component_layouts.begin(), component_layouts.end(), [](const auto& a, const auto& b) -> bool { return a.info.size > b.info.size; });

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
                if (empty_blocks[j].size >= component_layouts[i].info.size) // If the block is big enough for the type
                {
                    const auto next_align_pos = next_multiple(component_layouts[i].info.align, empty_blocks[j].start);
                    const auto block_end      = empty_blocks[j].start + empty_blocks[j].size;

                    if (next_align_pos < block_end) // If the next alignment is before the end of the block
                    {
                        // If the remaining size of the block with alignment accounted for is large enough for this type
                        const auto size_remaining = block_end - next_align_pos;
                        if (size_remaining >= component_layouts[i].info.size)
                        { // The remaining size in the block is enough, we can fit this type into the empty_block
                            component_layouts[i].offset = next_align_pos;

                            { // With the type offset assigned, the empty_block needs to be split/removed
                                const auto type_end = component_layouts[i].offset + component_layouts[i].info.size;

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
            ASSERT(component_layouts[i].offset != 0 || i == 0, "Failed to set the position of ComponentID {} in the buffer.", component_layouts[i].info.ID)
        }

        std::sort(component_layouts.begin(), component_layouts.end(), [](const auto& a, const auto& b) -> bool { return a.offset < b.offset; });
        return component_layouts;
    }

    // A container of Entity objects and the components they own.
    // Every unique combination of components makes an Archetype which is a contiguouse store of all the ComponentTypes.
    // Storage is interfaced using Entity as a key.
    class Storage
    {
        // Archetype is defined as a unique combination of ComponentTypes. It is a non-templated class allowing any combination of unique types to be stored in its m_data at runtime.
        // The ComponentTypes are retrievable using getComponent and getComponentImpl as well as their 'Mutable' variants.
        // Every archetype stores its mBitset for matching ComponentTypes.
        // mComponentLayout sets out how those ComponentTypes are laid out in an ArchetypeInstanceID.
        struct Archetype
        {
            ComponentBitset mBitset;                  // The unique identifier for this archetype. Each bit corresponds to a ComponentType this archetype stores per ArchetypeInstanceID.
            std::vector<ComponentLayout> mComponents; // How the ComponentTypes are laid out in each instance of ArchetypeInstanceID.
            std::vector<Entity> mEntities;            // Entity at every ArchetypeInstanceID. Should be indexed only using ArchetypeInstanceID.
            size_t mInstanceSize;                     // Size in Bytes of each archetype instance. In other words, the stride between every ArchetypeInstanceID.
            ArchetypeInstanceID mNextInstanceID;      // The ArchetypeInstanceID past the end of the m_data. Equivalant to size() in a vector.
            ArchetypeInstanceID m_capacity;           // The ArchetypeInstanceID count of how much memory is allocated in m_data for storage of components.
            std::byte* m_data;

            // Construct an Archetype from a template list of ComponentTypes.
            template<typename... ComponentTypes>
            Archetype(Meta::PackArgs<ComponentTypes...>)
                : mBitset{ComponentHelper::get_component_bitset<ComponentTypes...>()}
                , mComponents{get_components_layout<ComponentTypes...>()}
                , mEntities{}
                , mInstanceSize{get_stride(mComponents)}
                , mNextInstanceID{0}
                , m_capacity{Archetype_Start_Capacity}
                , m_data{(std::byte*)malloc(mInstanceSize * m_capacity)}

                    {
                LOG("[ECS] New Archetype created from components: {}", to_string(mComponents));
                    }

            // Search the mComponents vector for the ComponentType and return its ComponentLayout.
            template <typename ComponentType>
            const ComponentLayout& getComponentLayout() const
            {
                const auto component_ID = ComponentHelper::get_ID<ComponentType>();
                auto it = std::find_if(mComponents.begin(), mComponents.end(), [&component_ID](const auto& pComponentLayout)
                {
                    return pComponentLayout.info.ID == component_ID;
                });

                if (it != mComponents.end())
                    return *it;
                else
                {
                    throw std::logic_error("Requested a ComponentLayout for a ComponentType not present in this archetype.");
                    return mComponents.front();
                }
            };
            // Get the byte offset of ComponentType from the start of any ArchetypeInstanceID.
            template <typename ComponentType>
            BufferPosition getComponentOffset() const
            {
                const auto& layout = getComponentLayout<ComponentType>();
                return layout.offset;
            }
            // Get the byte position of ComponentType at ArchetypeInstanceID.
            template <typename ComponentType>
            BufferPosition getComponentPosition(const ArchetypeInstanceID& pInstanceIndex) const
            {
                const auto indexStartPosition = mInstanceSize * pInstanceIndex; // Position of the start of the instance at pInstanceIndex.
                return indexStartPosition + getComponentOffset<ComponentType>();
            }

            // Returns a const pointer to the ComponentType at pInstanceIndex.
            // The position of this component is found using a linear search of mComponentLayouts. If the BufferPosition is known use reinterpret_cast directly.
            template <typename ComponentType>
            const std::decay_t<ComponentType>* getComponent(const ArchetypeInstanceID& pInstanceIndex) const
            {
                const auto component_position = getComponentPosition<ComponentType>(pInstanceIndex);
                return reinterpret_cast<const std::decay_t<ComponentType>*>(&m_data[component_position]);
            }
            // Returns a pointer to the ComponentType at pInstanceIndex.
            // The position of this component is found using a linear search of mComponentLayouts. If the BufferPosition is known use reinterpret_cast directly.
            template <typename ComponentType>
            std::decay_t<ComponentType>* getComponentMutable(const ArchetypeInstanceID& pInstanceIndex)
            {
                const auto component_position = getComponentPosition<ComponentType>(pInstanceIndex);
                return reinterpret_cast<std::decay_t<ComponentType>*>(&m_data[component_position]);
            }

            // Inserts the components from the provided paramater pack ComponentTypes into the Archetype at the end.
            // If the archetype is full, more memory is reserved increasing the Archetype capacity.
            template <typename... ComponentTypes>
            void push_back(const Entity& pEntity, ComponentTypes&&... pComponentValues)
            {
                static_assert(Meta::is_unique<ComponentTypes...>, "Non unique component types! Archetype can only push back a set of unique ComponentTypes");

                if (mNextInstanceID + 1 > m_capacity)
                    reserve(next_greater_power_of_2(m_capacity));

                // Each `ComponentType` in the parameter pack is placement-new move-constructed into m_data
                auto construct_func = [&](auto&& p_component)
                {
                    using ComponentType = std::decay_t<decltype(p_component)>;

                    const auto componentStartPosition = getComponentPosition<ComponentType>(mNextInstanceID);
                    new (&m_data[componentStartPosition]) ComponentType(std::move(p_component));
                };
                (construct_func(std::move(pComponentValues)), ...); // Unfold construct_func over the ComponentTypes

                mEntities.push_back(pEntity);
                mNextInstanceID++;
            }

            // Remove the instance of the archetype at p_erase_index.
            void erase(const ArchetypeInstanceID& p_erase_index)
            {
                if (p_erase_index >= mNextInstanceID) throw std::out_of_range("Index out of range");

                const auto lastInstanceStartPosition  = mInstanceSize * (mNextInstanceID - 1);  // Position of the start of the last instance.

                if (p_erase_index == mNextInstanceID - 1)
                { // If erasing off the end, call the destructors for all the components at the end index
                    for (size_t comp = 0; comp < mComponents.size(); comp++)
                    {
                        const auto lastInstanceCompStartPosition = lastInstanceStartPosition + mComponents[comp].offset;
                        const auto lastInstanceCompAddress = &m_data[lastInstanceCompStartPosition];
                        mComponents[comp].info.funcs.Destruct(lastInstanceCompAddress);
                    }

                    mEntities.pop_back();
                    mNextInstanceID--;
                }
                else // Erasing an index not on the end of the Archetype
                {
                    const auto eraseInstanceStartPosition = mInstanceSize * p_erase_index; // Position of the start of the instance at p_erase_index.

                    // Move-assign the end components into the p_erase_index then call the destructor on all the end elements.
                    for (size_t comp = 0; comp < mComponents.size(); comp++)
                    {
                        const auto lastInstanceCompStartPosition = lastInstanceStartPosition + mComponents[comp].offset;
                        const auto lastInstanceCompAddress = &m_data[lastInstanceCompStartPosition];

                        const auto eraseInstanceCompStartPosition = eraseInstanceStartPosition + mComponents[comp].offset;
                        const auto eraseInstanceCompAddress = &m_data[eraseInstanceCompStartPosition];

                        mComponents[comp].info.funcs.MoveAssign(lastInstanceCompAddress, eraseInstanceCompAddress);
                        mComponents[comp].info.funcs.Destruct(lastInstanceCompAddress);
                    }

                    // Move the end EntityID into the erased index, pop back below.
                    mEntities[p_erase_index] = std::move(mEntities[mEntities.size() - 1]);
                    mEntities.pop_back();
                    mNextInstanceID--;
                }
            }

            // Allocate the memory required for p_new_capacity archetype instances. The m_size of the archetype is unchanged.
            void reserve(const size_t& p_new_capacity)
            {
                if (p_new_capacity <= m_capacity)
                    return;

                m_capacity = p_new_capacity;
                std::byte* new_data = (std::byte*)malloc(mInstanceSize * m_capacity);

                // Placement-new move-construct the objects from this into the auxillary store.
                // Then call the destructor on the old instances that were moved.
                for (size_t i = 0; i < mNextInstanceID; i++)
                {
                    auto instance_start = mInstanceSize * i;

                    for (size_t comp = 0; comp < mComponents.size(); comp++)
                    {
                        auto comp_data_position = instance_start + mComponents[comp].offset;

                        mComponents[comp].info.funcs.MoveConstruct(&new_data[comp_data_position], &m_data[comp_data_position]);
                        mComponents[comp].info.funcs.Destruct(&m_data[comp_data_position]);
                    }
                }

                free(m_data);
                m_data = new_data;
            }

            // Take the p_entity components from p_from_archetype that fit in this Archetype and destruct the remaining.
            // Move constructs the existing components from p_from_archetype into this.
            void inherit_entity(const Entity& p_entity, Archetype& p_from_archetype)
            {
                //p_from_archetype.
            }
        }; // class Archetype

        EntityID mNextEntity = 0;
        std::vector<Archetype> mArchetypes;
        // Maps EntityID to a position pair [ index in mArchetypes, ArchetypeInstanceID in archetype ].
        // Nullopt here means the entity was deleted.
        std::vector<std::optional<std::pair<ArchetypeID, ArchetypeInstanceID>>> mEntityToArchetypeID;

        template <typename... FunctionArgs>
        struct FunctionHelper;
        template <typename... FunctionArgs>
        struct FunctionHelper<Meta::PackArgs<FunctionArgs...>>
        {
            static_assert(Meta::is_unique<FunctionArgs...>, "Cannot construct a FunctionHelper from a list of types with duplicates. Are you calling foreach with repeating parameters?");
            static_assert(sizeof...(FunctionArgs) > 0, "Cannot construct a FunctionHelper with 0 types, are you calling foreach with 0 params?");

            static ComponentBitset getBitset()
            {
                return ECS::ComponentHelper::get_component_bitset<FunctionArgs...>();
            }
            // Does this function take only one parameter of type Entity.
            constexpr static bool isEntityFunction()
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
            static void applyToArchetype(const Func& pFunction, Archetype& pArchetype)
            {
                const auto indexSequence = std::index_sequence_for<FunctionArgs...>{};
                const auto offsets = getOffsets(pArchetype, indexSequence);
                impl(pFunction, pArchetype, offsets, indexSequence);
            }

        private:
            // Given a pFunction and pArchetype, calls pFunction on every ArchetypeInstanceID supplying the ComponentTypes as arguments.
            // pArchetypeOffsets: The mapping of pFunction arguments to their offsets per ArchetypeInstanceID.
            // index_sequence:    Provides a mechanism to execute a fold expression to retrieve all the arguments from the Archetype.
            template <std::size_t... Is>
            static void impl(const Func& pFunction, Archetype& pArchetype, const std::array<BufferPosition, sizeof...(FunctionArgs)>& pArchetypeOffsets, const std::index_sequence<Is...>&)
            { // If we have reached this point we can guarantee pArchetype contains all the components in FunctionArgs.
                for (ArchetypeInstanceID i = 0; i < pArchetype.mNextInstanceID; i++)
                    pFunction(*getFromArchetype<FunctionArgs>(pArchetype, i, pArchetypeOffsets[Is])...);
            }

            // Get a ComponentType* from pArchetype at pIndex with pOffset.
            template <typename ComponentType>
            static std::decay_t<ComponentType>* getFromArchetype(Archetype& pArchetype, const ArchetypeInstanceID& pIndex, const size_t& pOffset)
            {
                if constexpr (std::is_same_v<Entity, std::decay_t<ComponentType>>)
                    return &pArchetype.mEntities[pIndex];
                else
                    return reinterpret_cast<std::decay_t<ComponentType>*>(&pArchetype.m_data[(pArchetype.mInstanceSize * pIndex) + pOffset]);
            }

            // Assign the Byte offset of the ComponentType in pArchetype into pOffsets at pIndex. Skips over Entity's encountered.
            template <typename ComponentType, std::size_t... Is>
            static void setOffset(std::array<BufferPosition, sizeof...(FunctionArgs)>& pOffsets, const size_t& pIndex, const Archetype& pArchetype)
            {
                if constexpr (!std::is_same_v<Entity, std::decay_t<ComponentType>>) // Ignore any Entity params supplied.
                    pOffsets[pIndex] = pArchetype.getComponentLayout<ComponentType>().offset;
            }

            // Construct an array of corresponding to the offset of each FunctionArgs into the archetype.
            // Entity types encountered will not be set but the index in the returned array will exist.
            template <std::size_t... Is>
            static std::array<BufferPosition, sizeof...(FunctionArgs)> getOffsets(const Archetype& pArchetype, const std::index_sequence<Is...>&)
            {
                std::array<BufferPosition, sizeof...(FunctionArgs)> offsets;
                (setOffset<FunctionArgs>(offsets, Is, pArchetype), ...);
                return offsets;
            }
        };

        // Find the ArchetypeID with the exact matching componentBitset.
        // Every Archetype has a unique bitset so we can guarantee only one exists.
        // Returns nullopt if this archtype hasnt been added to mArchetypes yet.
        std::optional<ArchetypeID> getMatchingArchetype(const ComponentBitset& pComponentBitset)
        {
            for (ArchetypeID i = 0; i < mArchetypes.size(); i++)
            {
                if (pComponentBitset == mArchetypes[i].mBitset)
                    return i;
            }

            return std::nullopt;
        };
        // Find the ArchetypeIDs of any Archetypes with the exact matching componentBitset or containing it.
        // Returns an empty vec if there are none.
        std::vector<ArchetypeID> getMatchingOrContainedArchetypes(const ComponentBitset& pComponentBitset)
        {
            std::vector<ArchetypeID> returnVec;

            for (ArchetypeID i = 0; i < mArchetypes.size(); i++)
            {
                if (pComponentBitset == mArchetypes[i].mBitset || ((pComponentBitset & mArchetypes[i].mBitset) == pComponentBitset))
                    returnVec.push_back(i);
            }

            return returnVec;
        };

    public:
        // Creates an Entity out of the ComponentTypes.
        // The ComponentTypes must all be unique, only one of each ComponentType can be owned by an Entity.
        // The ComponentTypes can be retrieved individually using getComponent or as a combination using foreach.
        template <typename... ComponentTypes>
        Entity addEntity(ComponentTypes&&... pComponents)
        {
            static_assert(Meta::is_unique<ComponentTypes...>, "addEntity non-unique list of components given.");

            const ComponentBitset bitset = ComponentHelper::get_component_bitset<ComponentTypes...>();
            auto archetypeID = getMatchingArchetype(bitset);

            if (!archetypeID)
            {// No matching archetype was found we add a new one for this ComponentBitset.
                ComponentHelper::set_infos<ComponentTypes...>();
                mArchetypes.push_back(Archetype(Meta::PackArgs<ComponentTypes...>()));
                archetypeID = mArchetypes.size() - 1;
            }

            const auto newEntity = Entity(mNextEntity++);
            auto& archetype = mArchetypes[archetypeID.value()];
            archetype.push_back(newEntity, std::forward<ComponentTypes>(pComponents)...);
            mEntityToArchetypeID.push_back(std::make_optional(std::make_pair(archetypeID.value(), archetype.mNextInstanceID - 1)));

            return newEntity;
        }
        // Removes pEntity from storage.
        // The associated Entity is then on invalid for invoking other Storage funcrions on.
        void deleteEntity(const Entity& pEntity)
        {
            const auto [archetype, eraseIndex] = *mEntityToArchetypeID[pEntity.ID];
            mArchetypes[archetype].erase(eraseIndex);

            { // Update Storage bookeeping after the remove (mEntityToArchetypeID)
                if (eraseIndex != mArchetypes[archetype].mNextInstanceID) // If we removed from the end, no need to update mEntityToArchetypeID
                {
                    // After Archetype::erase, the EntityID at eraseIndex is the previously back ArchetypeInstance entity.
                    // Assign the movedEntityID the correct index in the archetype (the erased entity index)
                    auto& movedEntityID = mArchetypes[archetype].mEntities[eraseIndex].ID;
                    mEntityToArchetypeID[movedEntityID]->second = std::move(mEntityToArchetypeID[pEntity.ID]->second);
                }

                // mEntityToArchetypeID is index alligned to EntityID's we cannot erase the index corresponding to pEntity, instead set to nullopt.
                mEntityToArchetypeID[pEntity.ID] = std::nullopt;
            }
        }

        // Calls Func on every Entity which owns all of the components arguments of pFunction.
        // pFunction can have any number of ComponentTypes but will only be called if the Entity owns all of the components or more.
        // An optional Entity param in function will be supplied the Entity which owns the ComponentTypes on each call of pFunction.
        template <typename Func>
        void foreach(const Func& pFunction)
        {
            using FunctionParameterPack = typename Meta::GetFunctionInformation<Func>::GetParameterPack;

            const auto functionBitset = FunctionHelper<FunctionParameterPack>::getBitset();
            const auto archetypeIDs   = getMatchingOrContainedArchetypes(functionBitset);

            if (!archetypeIDs.empty())
            {
                for (auto& archetypeID : archetypeIDs)
                {
                    if (mArchetypes[archetypeID].mNextInstanceID > 0)
                    {
                        ApplyFunction<Func, FunctionParameterPack>::applyToArchetype(pFunction, mArchetypes[archetypeID]);
                    }
                }
            }
        };
        // Calls Func on every EntityID. Func must have only one parameter of type ECS::EntityID otherwise wont compile.
        template <typename Func>
        void foreachEntity(const Func& pFunction)
        {
            using FunctionParameterPack = typename Meta::GetFunctionInformation<Func>::GetParameterPack;
            static_assert(FunctionHelper<FunctionParameterPack>::isEntityFunction(), "pFunction is not a function that takes only one argument of type ECS::Entity");

            if (mNextEntity > 0)
            {
                for (EntityID i = 0; i < mNextEntity; i++)
                {
                    auto ent = Entity(i);
                    pFunction(ent);
                }
            }
        };

        // Get a const reference to component of ComponentType belonging to Entity.
        // If Entity doesn't own one exception will be thrown. Owned ComponentTypes can be queried using hasComponents.
        template <typename ComponentType>
        const std::decay_t<ComponentType>& getComponent(const Entity& pEntity) const
        {
            const auto [archetype, index] = *mEntityToArchetypeID[pEntity.ID];
            return *mArchetypes[archetype].getComponent<ComponentType>(index);
        }

        // Get a reference to component of ComponentType belonging to Entity.
        // If Entity doesn't own one exception will be thrown. Owned ComponentTypes can be queried using hasComponents.
        template <typename ComponentType>
        std::decay_t<ComponentType>& getComponentMutable(const Entity& pEntity)
        {
            const auto [archetype, index] = *mEntityToArchetypeID[pEntity.ID];
            return *mArchetypes[archetype].getComponentMutable<ComponentType>(index);
        }

        // Check if Entity has been assigned all of the ComponentTypes queried.
        template <typename... ComponentTypes>
        bool hasComponents(const Entity& pEntity) const
        {
            static_assert(sizeof...(ComponentTypes) != 0, "Cannot query hasComponents with 0 types.");

            if constexpr (sizeof...(ComponentTypes) > 1)
            {// Grab the archetype bitset the entity belongs to and check if the ComponentTypes bitset matches or is a subset of it.
                const auto requestedBitset = ComponentHelper::get_component_bitset<ComponentTypes...>();
                const auto [archetype, index] = *mEntityToArchetypeID[pEntity.ID];
                const auto entityBitset = mArchetypes[archetype].mBitset;
                return (requestedBitset == entityBitset || ((requestedBitset & entityBitset) == requestedBitset));
            }
            else
            {// If we only have one requested ComponentType, we can skip the ComponentTypes bitset construction and test just the corresponding bit.
                typedef typename Meta::GetNth<0, ComponentTypes...>::Type ComponentType;
                const auto [archetype, index] = *mEntityToArchetypeID[pEntity.ID];
                return mArchetypes[archetype].mBitset.test(ComponentHelper::get_ID<ComponentType>());
            }
        }
    };
} // namespace ECS