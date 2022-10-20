#pragma once

#include "Entity.hpp"

#include "EventDispatcher.hpp"
#include "Logger.hpp"

#include <vector>
#include <unordered_map>

namespace ECS
{
    template <typename Component>
    class ComponentManager
    {
    public:
        bool Contains(const Entity& pEntity) const
        {
            return mEntityComponentIndexLookup.find(pEntity.mID) != mEntityComponentIndexLookup.end();
        }
        void ForEach(const std::function<void(const Component& pComponent)>& pFunction) const
        {
            for (const Component& component : mComponents)
                pFunction(component);
        }
        // Read-only option to check if the entity has this component and retrieve it in one step.
        const Component* GetComponent(const Entity& pEntity) const
        {
            auto it = mEntityComponentIndexLookup.find(pEntity.mID);
            if (it != mEntityComponentIndexLookup.end())
                return &mComponents[it->second];
            else
                return nullptr;
        }
        void Add(const Entity& pEntity, const Component& pComponent)
        {
            ZEPHYR_ASSERT(pEntity.mID != INVALID_ENTITY_ID, "Invalid entity not allowed to create components");
            ZEPHYR_ASSERT(mEntityComponentIndexLookup.find(pEntity.mID) == mEntityComponentIndexLookup.end(), "Only one of this component type is allowed per entity");
            ZEPHYR_ASSERT(mEntities.size() == mComponents.size() && mEntityComponentIndexLookup.size() == mComponents.size(), "Entity count must always be the same as the number of components");

            // New components are always pushed to the end so entity lookup table receives end location
            mEntityComponentIndexLookup[pEntity.mID] = mComponents.size();
            mComponents.push_back(pComponent);
            // Also push corresponding entity
            mEntities.push_back(pEntity.mID);
            mComponentAddedEvent.Dispatch(pEntity, mComponents.back());
        }
        void Remove(const Entity& pEntity)
        {
            auto it = lookup.find(pEntity);
            if (it != lookup.end())
            {
                // Directly index into components and entities array:
                const size_t index = it->second;
                const Entity entity = mEntities[index];

                if (index < mComponents.size() - 1)
                {
                    // Swap out the dead element with the last one:
                    mComponents[index] = std::move(mComponents.back()); // try to use move
                    mEntities[index] = mEntities.back();

                    // Update the lookup table:
                    mEntityComponentIndexLookup[mEntities[index]] = index;
                }

                // Shrink the container:
                mComponents.pop_back();
                mEntities.pop_back();
                mEntityComponentIndexLookup.erase(entity);
                mComponentRemovedEvent.Dispatch(entity);
            }
        }
        // Apply pFunction to every Component in the list.
        void ModifyForEach(const std::function<void(Component& pComponent)>& pFunction)
        {
            for (size_t i = 0; i < mComponents.size(); i++)
            {
                const Component before = mComponents[i];
                pFunction(mComponents[i]);

                if (before != mComponents[i])
                    mComponentChangedEvent.Dispatch(mEntities[i], mComponents[i]);
            }
        }
        // Apply pFunction to Component belonging to the pEntity. Returns true if a component for this entity existed and pFunction was executed.
        bool Modify(const Entity& pEntity, const std::function<void(Component& pComponent)>& pFunction)
        {
            auto it = mEntityComponentIndexLookup.find(pEntity.mID);
            if (it != mEntityComponentIndexLookup.end())
            {
                const Component before = mComponents[it->second];
                pFunction(mComponents[it->second]);

                if (before != mComponents[it->second])
                    mComponentChangedEvent.Dispatch(pEntity.mID, mComponents[it->second]);

                return true;
            }
            else
                return false;
        }

        Utility::EventDispatcher<const Entity&, const Component&> mComponentAddedEvent;
        Utility::EventDispatcher<const Entity&, const Component&> mComponentChangedEvent;
        Utility::EventDispatcher<const Entity&> mComponentRemovedEvent;
    private:
        std::vector<Component> mComponents;
        std::vector<EntityID> mEntities;
        std::unordered_map<EntityID, size_t> mEntityComponentIndexLookup;
    };
}