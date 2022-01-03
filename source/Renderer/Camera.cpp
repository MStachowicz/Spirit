#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(const glm::vec3& pPosition, const std::function<void(const glm::mat4&)>& pOnViewChangeCallback, const float& pYaw, const float& pPitch)
    : mPosition(pPosition)
    , mYaw(pYaw)
    , mPitch(pPitch)
    , mFront()  // ----|
    , mUp()     //     |<-- set in updateCameraVectors()
    , mRight()  //     |
    , mView()   // ----|
    , mOnViewChange(pOnViewChangeCallback)
    , mMovementSpeed(2.5f)
    , mMouseSensitivity(0.1f)
    , mZoom(45.0f)
{
    updateCameraVectors();
}

void Camera::move(const MoveDirection& pDirection)
{
    const float velocity = mMovementSpeed;
    if (pDirection == Forward)
        mPosition += mFront * velocity;
    if (pDirection == Backward)
        mPosition -= mFront * velocity;
    if (pDirection == Left)
        mPosition -= mRight * velocity;
    if (pDirection == Right)
        mPosition += mRight * velocity;

    mView = glm::lookAt(mPosition, mPosition + mFront, mUp);
    mOnViewChange(mView);
}

void Camera::ProcessMouseMove(const float& pXOffset, const float& pYOffset, const bool& pConstrainPitch)
{
    mYaw    += (pXOffset * mMouseSensitivity);
    mPitch  += (pYOffset * mMouseSensitivity);

    // make sure that when pPitch is out of bounds, screen doesn't get flipped
    if (pConstrainPitch)
    {
        if (mPitch > 89.0f)
            mPitch = 89.0f;
        if (mPitch < -89.0f)
            mPitch = -89.0f;
    }
    updateCameraVectors();
}

void Camera::processScroll(const float& pOffset)
{
    mZoom -= pOffset;
    if (mZoom < 1.0f)
        mZoom = 1.0f;
    if (mZoom > 45.0f)
        mZoom = 45.0f;
}

void Camera::updateCameraVectors()
{
    glm::vec3 front; // calculate the new mFront vector
    front.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
    front.y = sin(glm::radians(mPitch));
    front.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));
    mFront = glm::normalize(front);

    // also re-calculate the Right and Up vector
    mRight = glm::normalize(glm::cross(mFront, WorldUp)); // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    mUp = glm::normalize(glm::cross(mRight, mFront));

    mView = glm::lookAt(mPosition, mPosition + mFront, mUp);
    mOnViewChange(mView);
}