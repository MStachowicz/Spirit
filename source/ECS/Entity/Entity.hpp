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
            , mActive(true)
        {}

        const EntityID mID;
        bool mActive;
    };
}