#pragma once

#include "Logger.hpp"

#include <algorithm>
#include <bitset>
#include <iostream>
#include <optional>
#include <typeindex>
#include <typeinfo>
#include <vector>
#include <array>

#include "Meta.hpp"

namespace ECS
{
    typedef size_t EntityID; // Used to index into archetype mBuffer.
    typedef size_t ArchetypeID;
    typedef size_t ArchetypeInstanceID; // Per ArchetypeID ID per component archetype instance.
    typedef size_t ComponentID;         // Unique identifier for any type passed into ECSStorage.
    typedef std::bitset<32> ComponentBitset;
    typedef size_t BufferPosition; // Used to index into archetype mBuffer.
    typedef unsigned char Byte;

    class ComponentIDGenerator
    {
        static inline ComponentID counter = 0;

        template<typename ComponentType>
        static inline ComponentID perComponentTypeID = counter++;

    public:
        template<typename ComponentType>
        static ComponentID get()
        {
            typedef typename std::remove_reference_t<ComponentType> DecayedComponentType;

            //LOG_INFO("ECS: New Component encountered given ECS::ComponentID: {} | {} ({}) = size: {}B ", perComponentTypeID<DecayedComponentType>, std::type_index(typeid(DecayedComponentType)).name(), typeid(DecayedComponentType).raw_name(), sizeof(DecayedComponentType));
            return perComponentTypeID<DecayedComponentType>;
        }
    };

    class Storage
    {
        template <typename... ComponentTypes>
        static ComponentBitset getBitset()
        {
            ComponentBitset componentBitset;
            (componentBitset.set(ComponentIDGenerator::get<ComponentTypes>()), ...);
            return componentBitset;
        }

        struct Archetype
        {
            // Archetype stores an instance of this per component holding information about how the memory layout of mBuffer.
            // Component layouts are constructed in processComponentsIntoArchetypeLayout.
            struct ComponentLayout
            {
                ComponentLayout() : mComponentID(0), mOffset(0)
                {}
                ComponentLayout(const ComponentID& pComponentID, const BufferPosition& pOffset)
                : mComponentID(pComponentID)
                , mOffset(pOffset)
                {}

                ComponentID mComponentID;
                BufferPosition mOffset; // Byte offset from the start of the archetype instance to this componentType.
            };

            const ComponentBitset componentStoredBitset;         // The unique identifier for this archetype. Each bit corresponds to a component this archetype stores per instance.
            const std::vector<ComponentLayout> mComponentLayout; // How the components are laid out in each instance of the archetype. The .size() of this vector tells us the number of components in an instance.
            const size_t mInstanceSize;                          // Byte Size of each archetype instance.
            size_t mInstanceCount;                               // The number of mArchetypes instances stored in the mBuffer.
            std::vector<Byte> mBuffer;

            // Take the list of ComponentTypes and return an array of their ComponentLayouts.
            template <typename... ComponentTypes>
            static std::array<ComponentLayout, sizeof...(ComponentTypes)> processComponentsIntoArchetypeLayout()
            {
                // #Optimisation: sort pComponentOrder in order of size for best padding?
                // #Optimisation: Index the components by word size offsets.

                size_t i = 0;
                size_t byteOffset = 0;
                std::array<ComponentLayout, sizeof...(ComponentTypes)> componentLayout;
                (void(componentLayout[i++] = ComponentLayout(ComponentIDGenerator::get<ComponentTypes>(), (byteOffset += sizeof(ComponentTypes)) - sizeof(ComponentTypes))), ...);
                return componentLayout;
            }

            // There is no way to explicitly specify templates for a constructor, as you cannot name a constructor.
            // This "cheats" by taking advantage of type deduction giving us access to 'ComponentTypes...' in the constructor.
            template <typename... ComponentTypes>
            struct ArchetypeParameterPack {};

            template<typename... ComponentTypes>
            Archetype(ArchetypeParameterPack<ComponentTypes...>)
            : componentStoredBitset(getBitset<ComponentTypes...>())
            , mComponentLayout(Meta::makeVector(processComponentsIntoArchetypeLayout<ComponentTypes...>()))
            , mInstanceSize(Meta::sizeOfVariadic<ComponentTypes...>())
            , mInstanceCount(0)
            {
                mBuffer.resize(mInstanceSize); // Reserve enough space to push 1 instance.

                std::string components      = "";
                std::string componentLayout = "|";
                for (size_t i = 0; i < mComponentLayout.size(); i++)
                {
                    components.empty() ? components = std::to_string(mComponentLayout[i].mComponentID) : components += ", " + std::to_string(mComponentLayout[i].mComponentID);

                    size_t size = 0;
                    if (mComponentLayout.size() == 1)
                        size = mInstanceSize;
                    else if (i == 0)
                        size = mComponentLayout[1].mOffset;
                    else if (i == mComponentLayout.size() - 1)
                        size = mInstanceSize - mComponentLayout[i].mOffset;
                    else
                        size = mComponentLayout[i + 1].mOffset - mComponentLayout[i].mOffset;

                    for (size_t j = 0; j < size; j++)
                    {
                        componentLayout += std::to_string(mComponentLayout[i].mComponentID);
                    }
                    componentLayout += '|';
                }
                LOG_INFO("ECS: New Archetype created out of component combination ({}). Memory layout: {}", components, componentLayout);
            }

            template <typename ComponentType>
            const ComponentLayout& getComponentLayout() const
            {
                const auto componentID = ComponentIDGenerator::get<ComponentType>();
                auto it = std::find_if(mComponentLayout.begin(), mComponentLayout.end(), [&componentID](const auto& pComponentLayout)
                    { return pComponentLayout.mComponentID == componentID; });

                if (it != mComponentLayout.end())
                    return *it;
                else
                {
                    throw std::logic_error("Requested a ComponentLayout for a ComponentType not present in this archetype.");
                    return mComponentLayout.front();
                }
            };

            // Returns the component of ComponentType at pInstanceIndex in the mBuffer.
            // pComponentID is used to lookup the offset of the ComponentType in an instance of the archetype.
            template <typename ComponentType>
            ComponentType getComponent(const ArchetypeInstanceID& pInstanceIndex)
            {
                const auto componentStartPosition = getComponentPosition<ComponentType>(pInstanceIndex);
                return getComponentImpl<ComponentType>(componentStartPosition);
            }

            // Returns the component of ComponentType at pPosition in the mBuffer.
            // There is no guaratee pPosition points to memory of ComponentType. Only use this function when you are certain pPosition is correct.
            // A valid pPosition can be retrieved using getComponentOffset and/or getComponentPosition.
            template <typename ComponentType>
            ComponentType getComponentImpl(const BufferPosition& pPosition)
            {
                //typedef typename std::remove_reference_t<ComponentType> DecayedComponentType;
                return reinterpret_cast<ComponentType>(mBuffer[pPosition]);
            }

            // Get the byte position of ComponentType at ArchetypeInstanceID.
            template <typename ComponentType>
            BufferPosition getComponentPosition(const ArchetypeInstanceID& pInstanceIndex) const
            {
                const auto indexStartPosition = mInstanceSize * pInstanceIndex; // Position of the start of the instance at pInstanceIndex.
                return indexStartPosition + getComponentOffset<ComponentType>();
            }

            // Get the byte offset of ComponentType from the start of any ArchetypeInstanceID.
            template <typename ComponentType>
            BufferPosition getComponentOffset() const
            {
                const auto& layout = getComponentLayout<ComponentType>();
                return layout.mOffset;
            }

            template <typename... ComponentTypes>
            std::array<BufferPosition, sizeof...(ComponentTypes)> getComponentOffsets()
            {
                std::array<BufferPosition, sizeof...(ComponentTypes)> offsetsOfComponentTypes;
                size_t i = 0;
                (void(offsetsOfComponentTypes[i++] = getComponentOffset<ComponentTypes>()) , ...);
                return offsetsOfComponentTypes;
            }

            // Assign ComponentType the value pComponent at ArchetypeInstanceID.
            template <typename ComponentType>
            void assign(const ComponentType& pComponent, const ArchetypeInstanceID& pInstanceIndex)
            {
                if (mBuffer.capacity() < getComponentPosition<ComponentType>(pInstanceIndex) + sizeof(pComponent))
                    throw std::logic_error("Index out of range! Trying to assign to a component past the end of the archetype buffer.");

                auto& component = getComponent<ComponentType&>(pInstanceIndex);
                component = pComponent;
            }

            // Assign pComponents past the end of the mBuffer.
            template <typename... ComponentTypes>
            void push_back(ComponentTypes&&... pComponents)
            {
                static_assert(Meta::is_unique<ComponentTypes...>);
                const auto endInstanceStartPosition = mInstanceSize * mInstanceCount;

                // double the size of the mBuffer until there is enough space for a new instance of this archetype.
                while (endInstanceStartPosition + mInstanceSize > mBuffer.capacity())
                    mBuffer.resize(mBuffer.capacity() * 2);

                auto assignComponent = [&](auto& pComponent) { assign(pComponent, mInstanceCount); };
                (assignComponent(pComponents), ...);

                mInstanceCount++; // Only do this once for all the components
            }
        }; // class Archetype

        EntityID mNextEntity = 0;
        std::vector<Archetype> mArchetypes;
        std::vector<std::pair<ArchetypeID, ArchetypeInstanceID>> mEntityToArchetypeID; // Maps EntityID to an arc

        template <typename... FunctionArgs>
        struct FunctionHelper;
        template <typename... FunctionArgs>
        struct FunctionHelper<Meta::PackArgs<FunctionArgs...>>
        {
            static_assert(Meta::is_unique<FunctionArgs...>); // Cannot construct a bitset from a list of types with duplicates. Are you calling foreach with repeating parameters.
            static_assert(sizeof...(FunctionArgs) > 0);      // Cannot construct a bitset with 0 function parameters. Are you calling foreach with no parameters.

            static ComponentBitset getBitset()
            {
                return ECS::Storage::getBitset<FunctionArgs...>();
            }
        };

        template <typename... FunctionArgs>
        struct ApplyFunctionToArchetype;
        template <typename Func, typename... FunctionArgs>
        struct ApplyFunctionToArchetype<Func, Meta::PackArgs<FunctionArgs...>>
        {
            static void apply(const Func& pFunction, Archetype& pArchetype)
            {
                const auto componentOffsets = pArchetype.getComponentOffsets<FunctionArgs...>();

                impl(pFunction, pArchetype, componentOffsets, std::make_index_sequence<sizeof...(FunctionArgs)>{});
            }

        private:
            template <std::size_t... Is>
            static void impl(const Func& pFunction, Archetype& pArchetype, const std::array<BufferPosition, sizeof...(FunctionArgs)>& pArchetypeOffsets, std::index_sequence<Is...>)
            { // If we have reached this point we can guarantee pArchetype contains all the components in FunctionArgs.
                for (size_t i = 0; i < pArchetype.mInstanceCount; i++)
                    pFunction(pArchetype.getComponentImpl<FunctionArgs>((pArchetype.mInstanceSize * i) + pArchetypeOffsets[Is])...);
            }
        };

        // Find the ArchetypeID of the archetype with the exact matching componentBitset.
        // Every Archetype has a unique bitset so we can guarantee only one exists.
        // Returns nullopt if this archtype hasnt been added to mArchetypes yet.
        std::optional<ArchetypeID> getMatchingArchetype(const ComponentBitset& pComponentBitset)
        {
            for (ArchetypeID i = 0; i < mArchetypes.size(); i++)
            {
                if (pComponentBitset == mArchetypes[i].componentStoredBitset)
                    return i;
            }

            return std::nullopt;
        };
        // Find the ArchetypeIDs of any mArchetypes with the exact matching componentBitsetor contain it.
        // Returns an empty vec if there are none.
        std::vector<ArchetypeID> getMatchingOrContainedArchetypes(const ComponentBitset& pComponentBitset)
        {
            std::vector<ArchetypeID> returnVec;

            for (ArchetypeID i = 0; i < mArchetypes.size(); i++)
            {
                if (pComponentBitset == mArchetypes[i].componentStoredBitset || ((pComponentBitset & mArchetypes[i].componentStoredBitset) == pComponentBitset))
                    returnVec.push_back(i);
            }

            return returnVec;
        };

    public:
        template <typename... ComponentTypes>
        EntityID addEntity(ComponentTypes&&... pComponents)
        {
            // To construct a new entity, we analyse the ComponentTypes and build a ComponentBitset to match an archetype to it.
            // Once we have an archetype we push the Component data to the Archetype buffer and update the EntityArchetypeMap.
            static_assert(Meta::is_unique<ComponentTypes...>);

            std::bitset<32> componentsBitset = getBitset<ComponentTypes...>();

            auto archetypeID = getMatchingArchetype(componentsBitset);
            if (!archetypeID)
            {// No matching archetype was found we add a new one for this ComponentBitset.
                mArchetypes.push_back(Archetype(Archetype::ArchetypeParameterPack<ComponentTypes...>()));
                archetypeID = mArchetypes.size() - 1;
            }

            auto& archetype = mArchetypes[archetypeID.value()];
            archetype.push_back(std::forward<ComponentTypes>(pComponents)...);
            mEntityToArchetypeID.push_back({archetypeID.value(), archetype.mInstanceCount - 1});

            return mNextEntity++;
        }

        template <typename Func>
        void foreach(const Func& pFunction)
        {
            // Match functionBitset to an archetype and call apply on it.
            using FunctionParameterPack = typename Meta::GetFunctionInformation<Func>::GetParameterPack;

            const auto functionBitset = FunctionHelper<FunctionParameterPack>::getBitset();
            const auto archetypeIDs   = getMatchingOrContainedArchetypes(functionBitset);

            if (!archetypeIDs.empty())
            {
                for (auto& archetypeID : archetypeIDs)
                {
                    if (mArchetypes[archetypeID].mInstanceCount > 0)
                    {
                        ApplyFunctionToArchetype<Func, FunctionParameterPack>::apply(pFunction, mArchetypes[archetypeID]);
                    }
                }
            }
        };

        template <typename ComponentType>
        const ComponentType& getComponent(const EntityID& pEntity)
        {
            const auto [archetype, index] = mEntityToArchetypeID[pEntity];
            return mArchetypes[archetype].getComponent<ComponentType>(index);
        }

        template <typename... ComponentTypes>
        bool hasComponents(const EntityID& pEntity)
        {
            const auto requestedBitset = getBitset<ComponentTypes...>();
            const auto [archetype, index] = mEntityToArchetypeID[pEntity];
            const auto entityBitset = mArchetypes[archetype].componentStoredBitset;
            return (requestedBitset == entityBitset || ((requestedBitset & entityBitset) == requestedBitset));
        }
    };
} // namespace ECS