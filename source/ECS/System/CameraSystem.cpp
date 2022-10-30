#include "CameraSystem.hpp"

#include "EventDispatcher.hpp"
#include "ComponentManager.hpp"
#include "Camera.hpp"

namespace System
{
    CameraSystem::CameraSystem(ECS::ComponentManager<Component::Camera>& pCameras)
        : mCameras{pCameras}
        , mBackupCamera(glm::vec3(0.0f, 1.7f, 7.0f))
        , mPrimaryCameraEntityID{std::nullopt}
    {
        mCameras.mComponentAddedEvent.Subscribe(std::bind(&System::CameraSystem::onCameraAdded, this, std::placeholders::_1, std::placeholders::_2));
        mCameras.mComponentChangedEvent.Subscribe(std::bind(&System::CameraSystem::onCameraChanged, this, std::placeholders::_1, std::placeholders::_2));
        mCameras.mComponentRemovedEvent.Subscribe(std::bind(&System::CameraSystem::onCameraRemoved, this, std::placeholders::_1));
    }
    void CameraSystem::modifyPrimaryCamera(const std::function<void(Component::Camera& pCamera)>& pFunctionToApply)
    {
        if (mPrimaryCameraEntityID.has_value())
            mCameras.Modify(mPrimaryCameraEntityID.value(), pFunctionToApply);
        else
        {
            pFunctionToApply(mBackupCamera);
            mPrimaryCameraViewChanged.Dispatch(mBackupCamera.getViewMatrix());
            mPrimaryCameraViewPositionChanged.Dispatch(mBackupCamera.getPosition());
        }
    }
    const Component::Camera& CameraSystem::getPrimaryCamera() const
    {
        if (mPrimaryCameraEntityID.has_value())
            return *mCameras.GetComponent(mPrimaryCameraEntityID.value());
        else
            return mBackupCamera;
    }

    void CameraSystem::onCameraAdded(const ECS::Entity& pEntity, const Component::Camera& pCamera)
    {
        if (pCamera.mPrimaryCamera)
            mPrimaryCameraEntityID = pEntity.mID;

    }
    void CameraSystem::onCameraChanged(const ECS::Entity& pEntity, const Component::Camera& pCamera)
    {
        if (pEntity.mID == mPrimaryCameraEntityID)
        {
            if (!pCamera.mPrimaryCamera)
                removePrimaryCamera(pEntity);
            else
            {
                mPrimaryCameraViewChanged.Dispatch(pCamera.getViewMatrix());
                mPrimaryCameraViewPositionChanged.Dispatch(pCamera.getPosition());
            }
        }
    }
    void CameraSystem::onCameraRemoved(const ECS::Entity& pEntity)
    {
        if (pEntity.mID == mPrimaryCameraEntityID)
            removePrimaryCamera(pEntity);
    }

    void CameraSystem::removePrimaryCamera(const ECS::Entity& pEntity)
    {
        ZEPHYR_ASSERT(pEntity.mID == mPrimaryCameraEntityID, "Calling remove on an entity with camera component not primary.");

        mPrimaryCameraEntityID = std::nullopt;
        LOG_INFO("Entity {} camera component no longer the primary camera", pEntity.mID);
    }
} // namespace System