#include <cstdint>

namespace ECS
{
    typedef uint32_t Entity;
    static const Entity INVALID_ENTITY = 0;
    static Entity next = 0;

    Entity CreateEntity()
    {
        return ++next;
    }
}