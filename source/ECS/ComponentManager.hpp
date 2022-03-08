#pragma once

#include <vector>
#include <unordered_map>
#include "Logger.hpp"
#include "Entity.hpp"

namespace ECS
{
    template <typename Component>
    class ComponentManager
    {
    public:
        inline bool Contains(const Entity &entity) const
        {
            return lookup.find(entity) != lookup.end();
        }

        inline size_t size() const
        {
            return components.size();
        }

        inline Component &operator[](const size_t& index)
        {
            return components[index];
        }

        inline Entity GetEntity(const size_t& index) const
        {
            return entities[index];
        }

        inline void ForEach(const std::function<void(const Component& pComponent)>& pFunction) const
        {
            for (size_t i = 0; i < size(); i++)
            {
                pFunction(components[i]);
            }
        }
        inline void ModifyForEach(const std::function<void(Component& pComponent)>& pFunction)
        {
            for (size_t i = 0; i < size(); i++)
            {
                pFunction(components[i]);
            }
        }

        inline const Component *GetComponent(const Entity &entity)
        {
            auto it = lookup.find(entity);
            if (it != lookup.end())
            {
                return &components[it->second];
            }
            else
                return nullptr;
        }

        Component& Create(const Entity &entity)
        {
            ZEPHYR_ASSERT(entity != INVALID_ENTITY, "Invalid entity not allowed to create components");
            ZEPHYR_ASSERT(lookup.find(entity) == lookup.end(), "Only one of this component type is allowed per entity");
            ZEPHYR_ASSERT(entities.size() == components.size() && lookup.size() == components.size(), "Entity count must always be the same as the number of components");

            // Update the entity lookup table:
            lookup[entity] = components.size();
            // New components are always pushed to the end:
            components.push_back(Component());
            // Also push corresponding entity:
            entities.push_back(entity);

            return components.back();
        }

        void Remove(const Entity &entity)
        {
            auto it = lookup.find(entity);
            if (it != lookup.end())
            {
                // Directly index into components and entities array:
                const size_t index = it->second;
                const Entity entity = entities[index];

                if (index < components.size() - 1)
                {
                    // Swap out the dead element with the last one:
                    components[index] = std::move(components.back()); // try to use move
                    entities[index] = entities.back();

                    // Update the lookup table:
                    lookup[entities[index]] = index;
                }

                // Shrink the container:
                components.pop_back();
                entities.pop_back();
                lookup.erase(entity);
            }
        }

    private:
        std::vector<Component> components;
        std::vector<Entity> entities;
        std::unordered_map<Entity, size_t> lookup;
    };
}