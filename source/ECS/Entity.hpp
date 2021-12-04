#pragma once

#include <cstdint>

namespace ECS
{
    typedef uint32_t Entity;
    static const Entity INVALID_ENTITY = 0;
    static Entity next = 0;

    inline Entity CreateEntity()
    {
        return ++next;
    }
}