#pragma once

#include <cstdint>

namespace ECS
{
    typedef size_t EntityID;

    class Entity
    {
    public:
        Entity(const EntityID pID)
            : mID(pID)
        {}

        const EntityID mID;
    };

    static const EntityID INVALID_ENTITY_ID = 0;
}