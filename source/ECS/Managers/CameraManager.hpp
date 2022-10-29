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
namespace Data
{
    class Camera;
}

namespace Manager
{
    class CameraManager
    {
    private:
        ECS::ComponentManager<Data::Camera>& mCameras;
        std::optional<ECS::EntityID> mPrimaryCameraEntityID;
        Data::Camera mBackupCamera; // If there is no mPrimaryCameraEntityID to use as primary camera, use this one.
    public:
        Utility::EventDispatcher<const glm::mat4&> mPrimaryCameraViewChanged;
        Utility::EventDispatcher<const glm::vec3&> mPrimaryCameraViewPositionChanged;

    public:
        CameraManager(ECS::ComponentManager<Data::Camera>& pCameras);
        void modifyPrimaryCamera(const std::function<void (Data::Camera& pCamera)>& pFunctionToApply);
        const Data::Camera& getPrimaryCamera() const;
    private:
        void onCameraAdded(const ECS::Entity& pEntity, const Data::Camera& pCamera);
        void onCameraChanged(const ECS::Entity& pEntity, const Data::Camera& pCamera);
        void onCameraRemoved(const ECS::Entity& pEntity);

        void removePrimaryCamera(const ECS::Entity& pEntity);
    };
} // namespace Manager