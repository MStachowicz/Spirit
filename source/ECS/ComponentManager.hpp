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
            ZEPHYR_ASSERT(mEntityComponentIndexLookup.find(pEntity.mID) == mEntityComponentIndexLookup.end(), "Only one of this component type is allowed per entity");
            ZEPHYR_ASSERT(mEntities.size() == mComponents.size() && mEntityComponentIndexLookup.size() == mComponents.size(), "Entity count must always be the same as the number of components");

            // New components are always pushed to the end so entity lookup table receives end location
            mEntityComponentIndexLookup[pEntity.mID] = mComponents.size();
            mComponents.push_back(pComponent);
            mEntities.push_back(pEntity.mID);
            mComponentAddedEvent.Dispatch(pEntity, mComponents.back());
        }
        void Remove(const Entity& pEntity)
        {
            // Find the index of the entity we are removing and swap it with the last entity then pop back.
            // We std::move the data of the back component/entity into the entityBeingRemovedIndex and then delete the duplicate off the end.
            auto it = mEntityComponentIndexLookup.find(pEntity.mID);
            if (it != mEntityComponentIndexLookup.end())
            {
                const size_t entityBeingRemovedIndex = it->second;
                ZEPHYR_ASSERT(mEntities[entityBeingRemovedIndex] == pEntity.mID, "Entity ID should match")

                mEntityComponentIndexLookup.erase(it);
                if (entityBeingRemovedIndex < mComponents.size() - 1)
                {
                    mEntityComponentIndexLookup[mEntities.back()] = entityBeingRemovedIndex;
                    mComponents[entityBeingRemovedIndex] = std::move(mComponents.back()); // Move the back entity data into the removed index.
                    mEntities[entityBeingRemovedIndex] = mEntities.back();
                }

                mComponents.pop_back();
                mEntities.pop_back();
                mComponentRemovedEvent.Dispatch(pEntity);
            }

            ZEPHYR_ASSERT(mEntityComponentIndexLookup.find(pEntity.mID) == mEntityComponentIndexLookup.end(), "Component still exists after remove.");
            ZEPHYR_ASSERT(mEntities.size() == mComponents.size() && mEntityComponentIndexLookup.size() == mComponents.size(), "Entity count must always be the same as the number of components");
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
        std::unordered_map<EntityID, size_t> mEntityComponentIndexLookup; // Maps EntityID to the index into mComponents and mEntities.
    };
}