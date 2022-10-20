#include "GraphicsAPI.hpp"

#include "Logger.hpp"

void GraphicsAPI::onEntityCreated(const ECS::Entity& pEntity, const ECS::EntityManager& pManager)
{
    ZEPHYR_ASSERT_MSG("Not implemented");
}
void GraphicsAPI::onEntityRemoved(const ECS::Entity& pEntity, const ECS::EntityManager& pManager)
{
    ZEPHYR_ASSERT_MSG("Not implemented");
}
void GraphicsAPI::onTransformComponentAdded(const ECS::Entity& pEntity, const Data::Transform& pTransform)
{
    ZEPHYR_ASSERT_MSG("Not implemented");
}
void GraphicsAPI::onTransformComponentChanged(const ECS::Entity& pEntity, const Data::Transform& pTransform)
{
    ZEPHYR_ASSERT_MSG("Not implemented");
}
void GraphicsAPI::onTransformComponentRemoved(const ECS::Entity& pEntity)
{
    ZEPHYR_ASSERT_MSG("Not implemented");
}
void GraphicsAPI::onMeshComponentAdded(const ECS::Entity& pEntity, const Data::MeshDraw& pMeshDraw)
{
    ZEPHYR_ASSERT_MSG("Not implemented");
}
void GraphicsAPI::onMeshComponentChanged(const ECS::Entity& pEntity, const Data::MeshDraw& pMeshDraw)
{
    ZEPHYR_ASSERT_MSG("Not implemented");
}
void GraphicsAPI::onMeshComponentRemoved(const ECS::Entity& pEntity)
{
    ZEPHYR_ASSERT_MSG("Not implemented");
}