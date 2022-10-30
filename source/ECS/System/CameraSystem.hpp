#pragma once

#include "Entity.hpp"
#include "Camera.hpp"

#include "EventDispatcher.hpp"

#include <optional>

namespace ECS
{
    template <class>
    class ComponentManager;
}
namespace Component
{
    class Camera;
}

namespace System
{
    class CameraSystem
    {
    private:
        ECS::ComponentManager<Component::Camera>& mCameras;
        std::optional<ECS::EntityID> mPrimaryCameraEntityID;
        Component::Camera mBackupCamera; // If there is no mPrimaryCameraEntityID to use as primary camera, use this one.
    public:
        Utility::EventDispatcher<const glm::mat4&> mPrimaryCameraViewChanged;
        Utility::EventDispatcher<const glm::vec3&> mPrimaryCameraViewPositionChanged;

    public:
        CameraSystem(ECS::ComponentManager<Component::Camera>& pCameras);
        void modifyPrimaryCamera(const std::function<void (Component::Camera& pCamera)>& pFunctionToApply);
        const Component::Camera& getPrimaryCamera() const;
    private:
        void onCameraAdded(const ECS::Entity& pEntity, const Component::Camera& pCamera);
        void onCameraChanged(const ECS::Entity& pEntity, const Component::Camera& pCamera);
        void onCameraRemoved(const ECS::Entity& pEntity);

        void removePrimaryCamera(const ECS::Entity& pEntity);
    };
} // namespace System