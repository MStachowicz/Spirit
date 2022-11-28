#pragma once

namespace ECS
{
    class Storage;
}
namespace System
{
    class MeshSystem;
}

namespace System
{
    class CollisionSystem
    {
       private:
        ECS::Storage& mStorage;
        const System::MeshSystem& mMeshSystem;

       public:
        CollisionSystem(ECS::Storage& pStorage, const System::MeshSystem& pMeshSystem);
        void checkCollisions();

    };
} // namespace System