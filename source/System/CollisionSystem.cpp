#include "CollisionSystem.hpp"

namespace System
{
    CollisionSystem::CollisionSystem(ECS::Storage& pStorage)
        : mStorage{pStorage}
    { }
} // namespace System