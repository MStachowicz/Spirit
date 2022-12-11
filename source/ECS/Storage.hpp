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
            typedef typename std::decay_t<ComponentType> DecayedComponentType;

            //LOG_INFO("ECS: New Component encountered given ECS::ComponentID: {} | {} ({}) = size: {}B ", perComponentTypeID<DecayedComponentType>, std::type_index(typeid(DecayedComponentType)).name(), typeid(DecayedComponentType).raw_name(), sizeof(DecayedComponentType));
            return perComponentTypeID<DecayedComponentType>;
        }
    };

    // Generates a bitset out of all the ComponentTypes.
    template <typename... ComponentTypes>
    static ComponentBitset getBitset()
    {
        ComponentBitset componentBitset;
        (componentBitset.set(ComponentIDGenerator::get<ComponentTypes>()), ...);
        return componentBitset;
    }

    // Creates a storage container for EntityID and Components.
    // Components are stored in Archetypes contiguously in memory for every unique combination of ComponentTypes.
    class Storage
    {
        // Archetype is defined as a unique combination of ComponentTypes. It is a non-templated class allowing any combination of unique types to be stored in its mBuffer.
        // The ComponentTypes are retrievable using getComponent and getComponentImpl as well as their 'Mutable' variants.
        // Every archetype stores its mBitset for matching ComponentTypes to. mComponentLayout sets out how those ComponentTypes are laid out in mBuffer.
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

            const ComponentBitset mBitset;         // The unique identifier for this archetype. Each bit corresponds to a ComponentType this archetype stores per ArchetypeInstanceID.
            const std::vector<ComponentLayout> mComponentLayout; // How the components are laid out in each ArchetypeInstanceID. The .size() of this vector tells us the number of unique ComponentTypes in an ArchetypeInstanceID.
            const size_t mInstanceSize;                          // Size in Bytes of each archetype instance. In other words, the stride between every ArchetypeInstanceID.
            ArchetypeInstanceID mNextInstanceID;                 // The ArchetypeInstanceID past the end of the mBuffer.
            std::vector<Byte> mBuffer;
            std::vector<EntityID> mEntities;                     // EntityID at every ArchetypeInstanceID. Should be indexed only using ArchetypeInstanceID.

            // Take the list of ComponentTypes and returns an array of their ComponentLayouts.
            // Packs the ComponentTypes tightly with 0 gaps in the same order they come in through the variadic template.
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
            // Construct an Archetype from a list of ComponentTypes. The order the ComponentTypes will be packed in mBuffer is determined in processComponentsIntoArchetypeLayout() function.
            template<typename... ComponentTypes>
            Archetype(ArchetypeParameterPack<ComponentTypes...>)
            : mBitset(getBitset<ComponentTypes...>())
            , mComponentLayout(Meta::makeVector(processComponentsIntoArchetypeLayout<ComponentTypes...>()))
            , mInstanceSize(Meta::sizeOfVariadic<ComponentTypes...>())
            , mNextInstanceID(0)
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

            // Search the mComponentLayout vector for the ComponentType and return its ComponentLayout.
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
            // Get the byte offset of ComponentType from the start of any ArchetypeInstanceID.
            template <typename ComponentType>
            BufferPosition getComponentOffset() const
            {
                const auto& layout = getComponentLayout<ComponentType>();
                return layout.mOffset;
            }
            // Get the byte position of ComponentType at ArchetypeInstanceID.
            template <typename ComponentType>
            BufferPosition getComponentPosition(const ArchetypeInstanceID& pInstanceIndex) const
            {
                const auto indexStartPosition = mInstanceSize * pInstanceIndex; // Position of the start of the instance at pInstanceIndex.
                return indexStartPosition + getComponentOffset<ComponentType>();
            }

            // Returns a const pointer to a component of ComponentType at pInstanceIndex in the mBuffer.
            // The position of this component is found using a linear search of mComponentLayouts. If the BufferPosition is known use getComponentImpl directly.
            template <typename ComponentType>
            const std::decay_t<ComponentType>* getComponent(const ArchetypeInstanceID& pInstanceIndex) const
            {
                const auto componentStartPosition = getComponentPosition<ComponentType>(pInstanceIndex);
                return getComponentImpl<ComponentType>(componentStartPosition);
            }
            // Returns a const pointer to a component of ComponentType at pPosition in the mBuffer.
            // There is no guaratee pPosition points to the start of a type ComponentType. Only use this function when you are certain pPosition is the start of a ComponentType!
            // A valid pPosition can be retrieved using getComponentPosition.
            template <typename ComponentType>
            const std::decay_t<ComponentType>* getComponentImpl(const BufferPosition& pPosition) const
            {
                return reinterpret_cast<const std::decay_t<ComponentType>*>(&mBuffer[pPosition]);
            }

            // Returns a pointer to a component of ComponentType at pPosition in the mBuffer.
            // There is no guaratee pPosition points to the start of a type ComponentType. Only use this function when you are certain pPosition is the start of a ComponentType!
            // A valid pPosition can be retrieved using getComponentPosition.
            template <typename ComponentType>
            std::decay_t<ComponentType>* getComponentMutable(const ArchetypeInstanceID& pInstanceIndex)
            {
                const auto componentStartPosition = getComponentPosition<ComponentType>(pInstanceIndex);
                return getComponentMutableImpl<ComponentType>(componentStartPosition);
            }
            // Returns a pointer to a component of ComponentType at pPosition in the mBuffer.
            // There is no guaratee pPosition points to the start of a type ComponentType. Only use this function when you are certain pPosition is the start of a ComponentType!
            // A valid pPosition can be retrieved using getComponentPosition.
            template <typename ComponentType>
            std::decay_t<ComponentType>* getComponentMutableImpl(const BufferPosition& pPosition)
            {
                return reinterpret_cast<std::decay_t<ComponentType>*>(&mBuffer[pPosition]);
            }

            // Assign pNewValue to the ComponentType at ArchetypeInstanceID.
            template <typename ComponentType>
            void assign(const ComponentType& pNewValue, const ArchetypeInstanceID& pInstanceIndex)
            {
                if (mBuffer.capacity() < getComponentPosition<ComponentType>(pInstanceIndex) + sizeof(pNewValue))
                    throw std::logic_error("Index out of range! Trying to assign to a component past the end of the archetype buffer.");

                auto* component = getComponentMutable<ComponentType>(pInstanceIndex);
                *component = pNewValue;
            }

            // Assign pComponentValues to a new ArchetypeInstanceID. The new ArchetypeInstanceID == mNextInstanceID.
            // If the new ArchetypeInstanceID extends beyond the end of mBuffer capacity, the buffer is doubled.
            template <typename... ComponentTypes>
            void push_back(const EntityID& pEntity, ComponentTypes&&... pComponentValues)
            {
                static_assert(Meta::is_unique<ComponentTypes...>);
                const auto endInstanceStartPosition = mInstanceSize * mNextInstanceID;

                // double the size of the mBuffer until there is enough space for a new instance of this archetype.
                while (endInstanceStartPosition + mInstanceSize > mBuffer.capacity())
                    mBuffer.resize(mBuffer.capacity() * 2);

                auto assignComponent = [&](auto& pComponent) { assign(pComponent, mNextInstanceID); };
                (assignComponent(pComponentValues), ...);

                mEntities.push_back(pEntity);
                mNextInstanceID++; // Only do this once for all the components
            }

            // Remove the instance of the archetype at pInstanceIndex.
            // This erase causes the end ArchetypeInstanceID EntityID to become the pInstanceIndex EntityID.
            void erase(const ArchetypeInstanceID& pInstanceIndex)
            {
                if (pInstanceIndex != mNextInstanceID - 1) // If erasing off the end, skip the move
                {
                    const auto eraseIndexStartPosition = mInstanceSize * pInstanceIndex; // Position of the start of the instance at pInstanceIndex.
                    const auto lastIndexStartPosition = mInstanceSize * (mNextInstanceID - 1); // Position of the start of the last instance.
                    const auto lastIndexEndPosition   = lastIndexStartPosition + mInstanceSize; // Position of the end of the last instance.

                    // The 'end' positions above are actually 1 index over the range, this is ok as std::move moves the range which contains all the elements
                    // between first and last, including the element pointed by first but not the element pointed by last.

                    // Move the end buffer data into the index requested for remove.
                    std::move(mBuffer.begin() + lastIndexStartPosition, mBuffer.begin() + lastIndexEndPosition, mBuffer.begin() + eraseIndexStartPosition);
                    // We don't pop mIntanceSize off the end of mBuffer as by decrementing mNextInstanceID the next push_back will overwrite the data anyway.
                    // Reducing mNextInstanceID will also prevent forEach iterating past the end of the valid mBuffer range.

                    mEntities[pInstanceIndex] = std::move(mEntities[mEntities.size() - 1]); // Move the end EntityID into the erased index, pop back below.
                }

                mNextInstanceID--;
                mEntities.pop_back();
            }

            template <typename... ComponentTypes>
            std::array<BufferPosition, sizeof...(ComponentTypes)> getComponentOffsets() const
            {
                std::array<BufferPosition, sizeof...(ComponentTypes)> offsetsOfComponentTypes;
                size_t i = 0;
                (void(offsetsOfComponentTypes[i++] = getComponentOffset<ComponentTypes>()), ...);
                return offsetsOfComponentTypes;
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
            static_assert(Meta::is_unique<FunctionArgs...>); // Cannot construct a bitset from a list of types with duplicates. Are you calling foreach with repeating parameters.
            static_assert(sizeof...(FunctionArgs) > 0);      // Cannot construct a bitset with 0 function parameters. Are you calling foreach with no parameters.

            static ComponentBitset getBitset()
            {
                return ECS::getBitset<FunctionArgs...>();
            }
            // Does this function take only one parameter of type EntityID.
            constexpr static bool isEntityIDFunction()
            {
                if constexpr(sizeof...(FunctionArgs) == 1 && std::is_same_v<EntityID, std::decay_t<typename Meta::GetNth<0, FunctionArgs...>::Type>>)
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
                const auto componentOffsets = pArchetype.getComponentOffsets<FunctionArgs...>();
                impl(pFunction, pArchetype, componentOffsets, std::make_index_sequence<sizeof...(FunctionArgs)>{});
            }

        private:
            template <std::size_t... Is>
            static void impl(const Func& pFunction, Archetype& pArchetype, const std::array<BufferPosition, sizeof...(FunctionArgs)>& pArchetypeOffsets, std::index_sequence<Is...>)
            { // If we have reached this point we can guarantee pArchetype contains all the components in FunctionArgs.
                for (size_t i = 0; i < pArchetype.mNextInstanceID; i++)
                    pFunction(*pArchetype.getComponentMutableImpl<FunctionArgs>((pArchetype.mInstanceSize * i) + pArchetypeOffsets[Is])...);
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
        // Creates an EntityID out of the ComponentTypes.
        // The ComponentTypes must all be unique, only one of each ComponentType can be owned by an EntityID.
        // The ComponentTypes can be retrieved individually using getComponent or as a combination using foreach.
        template <typename... ComponentTypes>
        EntityID addEntity(ComponentTypes&&... pComponents)
        {
            static_assert(Meta::is_unique<ComponentTypes...>);

            const std::bitset<32> componentsBitset = getBitset<ComponentTypes...>();
            auto archetypeID = getMatchingArchetype(componentsBitset);

            if (!archetypeID)
            {// No matching archetype was found we add a new one for this ComponentBitset.
                mArchetypes.push_back(Archetype(Archetype::ArchetypeParameterPack<ComponentTypes...>()));
                archetypeID = mArchetypes.size() - 1;
            }

            const auto newEntity = mNextEntity++;
            auto& archetype = mArchetypes[archetypeID.value()];
            archetype.push_back(newEntity, std::forward<ComponentTypes>(pComponents)...);
            mEntityToArchetypeID.push_back(std::make_optional(std::make_pair(archetypeID.value(), archetype.mNextInstanceID - 1)));

            return newEntity;
        }
        // Removes pEntity from storage.
        // The associated EntityID is then on invalid for invoking other Storage funcrions on.
        void deleteEntity(const EntityID& pEntity)
        {
            const auto [archetype, eraseIndex] = *mEntityToArchetypeID[pEntity];
            mArchetypes[archetype].erase(eraseIndex);

            { // Update Storage bookeeping after the remove (mEntityToArchetypeID)
                if (eraseIndex != mArchetypes[archetype].mNextInstanceID) // If we removed from the end, no need to update mEntityToArchetypeID
                {
                    // After Archetype::erase, the EntityID at eraseIndex is the previously back ArchetypeInstance entity.
                    // Assign the movedEntityID the correct index in the archetype (the erased entity index)
                    auto& movedEntityID = mArchetypes[archetype].mEntities[eraseIndex];
                    mEntityToArchetypeID[movedEntityID]->second = std::move(mEntityToArchetypeID[pEntity]->second);
                }

                // mEntityToArchetypeID is index alligned to EntityID's we cannot erase the index corresponding to pEntity, instead set to nullopt.
                mEntityToArchetypeID[pEntity] = std::nullopt;
            }
        }

        // Calls Func on every EntityID which owns all of the components in the Func parameter pack.
        // Func can have any number of ComponentTypes but will only be called if the Entity owns all of the components or more.
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
            static_assert(FunctionHelper<FunctionParameterPack>::isEntityIDFunction());

            if (mNextEntity > 0)
            {
                for (EntityID i = 0; i < mNextEntity; i++)
                    pFunction(i);
            }
        };

        // Get a const reference to component of ComponentType belonging to EntityID.
        // If EntityID doesn't own one exception will be thrown. Owned ComponentTypes can be queried using hasComponents.
        template <typename ComponentType>
        const std::decay_t<ComponentType>& getComponent(const EntityID& pEntity) const
        {
            const auto [archetype, index] = *mEntityToArchetypeID[pEntity];
            return *mArchetypes[archetype].getComponent<ComponentType>(index);
        }

        // Get a reference to component of ComponentType belonging to EntityID.
        // If EntityID doesn't own one exception will be thrown. Owned ComponentTypes can be queried using hasComponents.
        template <typename ComponentType>
        std::decay_t<ComponentType>& getComponentMutable(const EntityID& pEntity)
        {
            const auto [archetype, index] = *mEntityToArchetypeID[pEntity];
            return *mArchetypes[archetype].getComponentMutable<ComponentType>(index);
        }

        // Check if EntityID has been assigned all of the ComponentTypes queried.
        template <typename... ComponentTypes>
        bool hasComponents(const EntityID& pEntity) const
        {
            static_assert(sizeof...(ComponentTypes) != 0);

            if constexpr(sizeof...(ComponentTypes) > 1)
            {// Grab the archetype bitset the entity belongs to and check if the ComponentTypes bitset matches or is a subset of it.
                const auto requestedBitset = getBitset<ComponentTypes...>();
                const auto [archetype, index] = *mEntityToArchetypeID[pEntity];
                const auto entityBitset = mArchetypes[archetype].mBitset;
                return (requestedBitset == entityBitset || ((requestedBitset & entityBitset) == requestedBitset));
            }
            else
            {// If we only have one requested ComponentType, we can skip the ComponentTypes bitset construction and test just the corresponding bit.
                typedef typename Meta::GetNth<0, ComponentTypes...>::Type ComponentType;
                const auto [archetype, index] = *mEntityToArchetypeID[pEntity];
                return mArchetypes[archetype].mBitset.test(ComponentIDGenerator::get<ComponentType>());
            }
        }
    };
} // namespace ECS