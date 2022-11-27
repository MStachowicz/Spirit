#pragma once

namespace ECS
{
    class Storage;
}

namespace System
{
    class CollisionSystem
    {
       private:
        ECS::Storage& mStorage;

       public:
        CollisionSystem(ECS::Storage& pStorage);
    };
} // namespace System