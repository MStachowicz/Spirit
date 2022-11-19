#include "CollisionSystem.hpp"

namespace Collision
{
    CollisionSystem::CollisionSystem(ECS::Storage& pStorage)
        : mStorage{pStorage}
    { }
} // namespace Collision