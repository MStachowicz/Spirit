#pragma once

#include "Logger.hpp"

#include <algorithm>
#include <bitset>
#include <iostream>
#include <optional>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include "Meta.hpp"

namespace ECS
{
    typedef size_t EntityID; // Used to index into archetype mBuffer.
    typedef size_t ArchetypeID;
    typedef size_t ComponentID; // Unique identifier for any type passed into ECSStorage.
    typedef std::bitset<32> ComponentBitset;
    typedef size_t BufferPosition; // Used to index into archetype mBuffer.
    typedef unsigned char Byte;

    class Storage
    {
        struct Archetype
        {
            // Archetype stores an instance of this per component it stores to hold information about how its mBuffer layout can be accessed.
            struct ComponentLayout
            {
                ComponentID componentID;
                size_t size;           // Size in bytes of this ComponentType
                BufferPosition offset; // Byte offset from the start of the archetype instance to this componentType.
            };

            const ComponentBitset componentStoredBitset;         // The unique identifier for this archetype. Each bit corresponds to a component this archetype stores per instance.
            const std::vector<ComponentLayout> mComponentLayout; // How the components are laid out in each instance of the archetype.
            const size_t mInstanceSize;                          // Byte Size of each archetype instance.
            size_t mInstanceCount;                               // The number of archetypes instances stored in the mBuffer.
            std::vector<Byte> mBuffer;

            // Take the list of pComponents and return a ComponentLayout for each one.
            static std::vector<ComponentLayout> processComponentsIntoArchetypeLayout(const std::vector<std::pair<ComponentID, size_t>>& pComponents)
            {
                // #Optimisation: sort pComponentOrder in order of size for best padding?
                // #Optimisation: Index the components by word size offsets.

                std::vector<ComponentLayout> returnVec;
                size_t byteOffset = 0;

                for (size_t i = 0; i < pComponents.size(); i++)
                {
                    returnVec.push_back({pComponents[i].first, pComponents[i].second, byteOffset});
                    byteOffset += pComponents[i].second;
                }
                return returnVec;
            }

            Archetype(const ComponentBitset& pComponentBitset, const std::vector<std::pair<ComponentID, size_t>>& pComponentOrder, const size_t& pSumComponentsSize)
                : componentStoredBitset(pComponentBitset)
                , mComponentLayout{processComponentsIntoArchetypeLayout(pComponentOrder)}
                , mInstanceSize(pSumComponentsSize)
                , mInstanceCount(0)
            {
                mBuffer.resize(mInstanceSize); // Reserve enough space to push 1 instance.

                std::string components      = "";
                std::string componentLayout = "|";
                for (const auto& component : mComponentLayout)
                {
                    components.empty() ? components = std::to_string(component.componentID) : components += ", " + std::to_string(component.componentID);
                    for (size_t i = 0; i < component.size; i++)
                    {
                        componentLayout += std::to_string(component.componentID);
                    }
                    componentLayout += '|';
                }
                LOG_INFO("ECS: New Archetype created out of component combination ({}). Memory layout: {}", components, componentLayout);
            }

            const ComponentLayout& getComponentLayout(const ComponentID& pComponentID) const
            {
                auto it = std::find_if(mComponentLayout.begin(), mComponentLayout.end(), [&pComponentID](const auto& pComponentLayout)
                                       { return pComponentLayout.componentID == pComponentID; });

                if (it != mComponentLayout.end())
                    return *it;
                else
                {
                    ZEPHYR_ASSERT(false, "Requested layout for a ComponentID not present in the archetype.");
                    return mComponentLayout.front();
                }
            };

            // Returns the component of ComponentType at pInstanceIndex in the mBuffer.
            // pComponentID is used to lookup the offset of the ComponentType in an instance of the archetype.
            template <typename ComponentType>
            ComponentType getComponent(const size_t& pInstanceIndex, const ComponentID& pComponentID)
            {
                const auto componentStartPosition = getComponentPosition(pComponentID, pInstanceIndex);
                return getComponent<ComponentType>(componentStartPosition);
            }

            // Returns the component of ComponentType at pPosition in the mBuffer.
            // There is no guaratee pPosition points to memory of ComponentType. Only use this function when you are certain pPosition is correct.
            // A valid pPosition can be retrieved using getComponentOffset and/or getComponentPosition.
            template <typename ComponentType>
            ComponentType getComponent(const BufferPosition& pPosition)
            {
                return reinterpret_cast<ComponentType>(mBuffer[pPosition]);
            }

            // Get the byte position of ComponentID at archetype pInstanceIndex.
            BufferPosition getComponentPosition(const ComponentID& pComponentID, const size_t pInstanceIndex) const
            {
                const auto indexStartPosition = mInstanceSize * pInstanceIndex; // Position of the start of the instance at pInstanceIndex.
                return indexStartPosition + getComponentOffset(pComponentID);
            }

            // Get the byte offset of ComponentID from the start of any archetype instance.
            BufferPosition getComponentOffset(const ComponentID& pComponentID) const
            {
                const auto& layout = getComponentLayout(pComponentID);
                return layout.offset;
            }

            std::vector<BufferPosition> getComponentOffsets(const std::vector<ComponentID>& pComponentIDs)
            {
                std::vector<BufferPosition> offsets;
                offsets.reserve(pComponentIDs.size());

                for (const auto& componentID : pComponentIDs)
                    offsets.push_back(getComponentOffset(componentID));

                return offsets;
            }

            // Assign pComponent to the archetype instance at pIndex.
            template <typename ComponentType>
            void assign(const ComponentType& pComponent, const ComponentID& pComponentID, const size_t pInstanceIndex)
            {
                auto& component = getComponent<ComponentType&>(pInstanceIndex, pComponentID);
                ZEPHYR_ASSERT(mBuffer.capacity() >= getComponentPosition(pComponentID, pInstanceIndex) + sizeof(pComponent), "Index out of range. Buffer needs to be extended.");
                component = pComponent;
            }

            // Assign pComponents past the end of the mBuffer.
            template <typename... ComponentTypes>
            void push_back(const std::vector<std::pair<ComponentID, size_t>>& pComponentIDAndSizes, ComponentTypes&&... pComponents)
            {
                static_assert(Meta::is_unique<ComponentTypes...>);
                const auto endInstanceStartPosition = mInstanceSize * mInstanceCount;

                // double the size of the mBuffer until there is enough space for a new instance of this archetype.
                while (endInstanceStartPosition + mInstanceSize > mBuffer.capacity())
                    mBuffer.resize(mBuffer.capacity() * 2);

                size_t i             = 0;
                auto assignComponent = [&](auto& pComponent)
                {
                    assign(pComponent, pComponentIDAndSizes[i].first, mInstanceCount);
                    i++;
                };
                (assignComponent(pComponents), ...);

                mInstanceCount++; // Only do this once for all the components
            }
        }; // class Archetype

        std::unordered_map<std::type_index, ComponentID> typeToComponentID; // Maps a type passed into the ECSStorage to its unique ComponentID.
        ComponentID nextComponentID = 0;
        EntityID nextEntity         = 0;
        std::vector<Archetype> archetypes;

        static ComponentBitset getBitset(const std::vector<ComponentID> pComponentIDs)
        {
            std::bitset<32> returnBitset;

            for (const auto componentID : pComponentIDs)
                returnBitset.set(componentID);

            return returnBitset;
        }

        // Find the ComponentID for the requested type. If one does not exist append it to the typeToComponentID map.
        template <typename ComponentType>
        ComponentID getComponentID(const ComponentType& pComponent)
        {
            const auto& typeID   = typeid(pComponent);
            const auto typeIndex = std::type_index(typeID);

            auto it = typeToComponentID.find((typeIndex));
            if (it == typeToComponentID.end())
            {
                typeToComponentID[typeIndex] = nextComponentID++;
                LOG_INFO("ECS: new Component encountered {} = size: {}B | ECS::ID: {}", typeID.name(), sizeof(ComponentType), nextComponentID - 1);
                return nextComponentID - 1;
            }
            else
                return it->second;
        };

        template <typename... FunctionArgs>
        struct FunctionHelper;
        template <typename... FunctionArgs>
        struct FunctionHelper<Meta::PackArgs<FunctionArgs...>>
        {
            static_assert(Meta::is_unique<FunctionArgs...>); // Cannot construct a bitset from a list of types with duplicates. Are you calling foreach with repeating parameters.
            static_assert(sizeof...(FunctionArgs) > 0);      // Cannot construct a bitset with 0 function parameters. Are you calling foreach with no parameters.

            template <typename ComponentType>
            static std::optional<ComponentID> getComponentIDFromMap(const std::unordered_map<std::type_index, ComponentID>& map)
            {
                const auto& typeID   = typeid(ComponentType);
                const auto typeIndex = std::type_index(typeID);
                auto it              = map.find(typeIndex);

                if (it == map.end())
                    return std::nullopt;
                else
                    return it->second;
            }
            template <typename Component>
            static bool appendToVector(std::vector<ComponentID>& pComponentVec, const std::unordered_map<std::type_index, ComponentID>& pMap)
            {
                auto ID = getComponentIDFromMap<Component>(pMap);
                if (ID.has_value())
                {
                    pComponentVec.push_back(ID.value());
                    return true;
                }
                else
                {
                    ZEPHYR_ASSERT(false, "ECSStorage hasnt encountered type '{}' before, invalid vector will be constructed", typeid(Component).name())
                    return false;
                }
            }
            static std::vector<ComponentID> getComponentIDs(const std::unordered_map<std::type_index, ComponentID>& pMap)
            {
                std::vector<ComponentID> componentIDs;
                (appendToVector<FunctionArgs>(componentIDs, pMap), ...); // Optimisation: Early out here if appendToVector returns false we know there cant be a matching archetype later
                return componentIDs;
            }
        };

        template <typename... FunctionArgs>
        struct ApplyFunctionToArchetype;
        template <typename Func, typename... FunctionArgs>
        struct ApplyFunctionToArchetype<Func, Meta::PackArgs<FunctionArgs...>>
        {
            static void apply(const Func& pFunction, Archetype& pArchetype, const std::vector<BufferPosition>& pArchetypeOffsets)
            {
                impl(pFunction, pArchetype, pArchetypeOffsets, std::make_index_sequence<sizeof...(FunctionArgs)>{});
            }

        private:
            template <std::size_t... Is>
            static void impl(const Func& pFunction, Archetype& pArchetype, const std::vector<BufferPosition>& pArchetypeOffsets, std::index_sequence<Is...>)
            { // If we have reached this point we can guarantee pArchetype contains all the components in FunctionArgs.
                for (size_t i = 0; i < pArchetype.mInstanceCount; i++)
                    pFunction(pArchetype.getComponent<FunctionArgs>((pArchetype.mInstanceSize * i) + pArchetypeOffsets[Is])...);
            }
        };

        // Find the ArchetypeID of the archetype with the exact matching componentBitset.
        // Every Archetype has a unique bitset so we can guarantee only one exists.
        // Returns nullopt if this archtype hasnt been added to mArchetypes yet.
        std::optional<ArchetypeID> getMatchingArchetype(const ComponentBitset& pComponentBitset)
        {
            for (ArchetypeID i = 0; i < archetypes.size(); i++)
            {
                if (pComponentBitset == archetypes[i].componentStoredBitset)
                    return i;
            }

            return std::nullopt;
        };
        // Find the ArchetypeIDs of any archetypes with the exact matching componentBitsetor contain it.
        // Returns an empty vec if there are none.
        std::vector<ArchetypeID> getMatchingOrContainedArchetypes(const ComponentBitset& pComponentBitset)
        {
            std::vector<ArchetypeID> returnVec;

            for (ArchetypeID i = 0; i < archetypes.size(); i++)
            {
                if (pComponentBitset == archetypes[i].componentStoredBitset || ((pComponentBitset & archetypes[i].componentStoredBitset) == pComponentBitset))
                    returnVec.push_back(i);
            }

            return returnVec;
        };

    public:
        template <typename... ComponentTypes>
        EntityID addEntity(ComponentTypes&&... pComponents)
        {
            static_assert(Meta::is_unique<ComponentTypes...>);

            // #OPTIMISATION make this a std::array using size of pComponents pack
            std::vector<std::pair<ComponentID, size_t>> componentsInArguments;
            std::bitset<32> componentsBitset;
            size_t sumSize = 0;

            // For every component in the parameter pack, get/assign it a ComponentID and build up the information for finding an archetype home for the entity.
            auto processComponent = [&](auto& pComponent)
            {
                const auto ID = getComponentID(pComponent);
                componentsInArguments.push_back({ID, sizeof(pComponent)});
                componentsBitset.set(ID, true);
                sumSize += sizeof(pComponent);
            };
            (processComponent(pComponents), ...);

            auto archetypeID = getMatchingArchetype(componentsBitset);
            if (!archetypeID)
            {
                // No matching archetype was found we add a new one for this bitset.
                archetypes.push_back({componentsBitset, componentsInArguments, sumSize});
                archetypeID = archetypes.size() - 1;
            }

            // Push the ComponentTypes&&... pComponents to the archetypeID
            auto& archetype = archetypes[archetypeID.value()];
            archetype.push_back(componentsInArguments, std::forward<ComponentTypes>(pComponents)...);

            return nextEntity++;
        }

        template <typename Func>
        void foreach(const Func& pFunction)
        {
            // Match functionBitset to an archetype and call apply on it.
            using FunctionParameterPack = typename Meta::GetFunctionInformation<Func>::GetParameterPack;

            const auto functionComponentIDs = FunctionHelper<FunctionParameterPack>::getComponentIDs(typeToComponentID);
            const auto functionBitset       = getBitset(functionComponentIDs);
            const auto archetypeIDs         = getMatchingOrContainedArchetypes(functionBitset);

            if (!archetypeIDs.empty())
            {
                for (auto& archetypeID : archetypeIDs)
                {
                    if (archetypes[archetypeID].mInstanceCount > 0)
                    {
                        const auto componentOffsets = archetypes[archetypeID].getComponentOffsets(functionComponentIDs);
                        ApplyFunctionToArchetype<Func, FunctionParameterPack>::apply(pFunction, archetypes[archetypeID], componentOffsets);
                    }
                }
            }
        };
    };
} // namespace ECS