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
            return mEntityComponentIndexLookup.find(entity) != mEntityComponentIndexLookup.end();
        }
        void ForEach(const std::function<void(const Component& pComponent)>& pFunction) const
        {
            for (const Component& component : mComponents)
                pFunction(component);
        }
        const Component* GetComponent(const Entity& pEntity) const
        {
            auto it = mEntityComponentIndexLookup.find(pEntity.mID);
            if (it != mEntityComponentIndexLookup.end())
            {
                return &mComponents[it->second];
            }
            else
                return nullptr;
        }

        // Apply pFunction to every Component in the list.
        void ModifyForEach(const std::function<void(Component& pComponent)>& pFunction)
        {
            for (Component& component : mComponents)
            {
                const Component before = component;
                pFunction(component);

                if (before != component)
                    mChangedComponentEvent.Dispatch(component);
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
                    mChangedComponentEvent.Dispatch(mComponents[it->second]);

                return true;
            }
            else
                return false;
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
            }
        }

        Utility::EventDispatcher<const Component&> mChangedComponentEvent;
    private:
        std::vector<Component> mComponents;
        std::vector<EntityID> mEntities;
        std::unordered_map<EntityID, size_t> mEntityComponentIndexLookup;
    };
}