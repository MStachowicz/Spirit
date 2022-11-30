#pragma once

#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"

#include <functional>

namespace Component
{
    // Camera operating using Euler angles to calculate the corresponding Vectors and Matrices to define a view in 3D space.
    class Camera
    {
    public:
        Camera(const glm::vec3& pPosition, const float& pYaw = -90.0f, const float& pPitch = 0.0f);

        enum MoveDirection { Forward, Backward, Left, Right, Up, Down };
        void move(const MoveDirection& pDirection);                                                              // Process key events to move the Camera mPosition.
        void ProcessMouseMove(const float& pXOffset, const float& pYOffset, const bool& pConstrainPitch = true); // Processes mouse movement to allow moving camera in it's current mPosition.
        void processScroll(const float& pOffset);                                                                // Process mouse scrollwheel events. Applies a zoom on the camera.

        bool mPrimaryCamera; // Only one Component::Camera can be a primaryCamera, this camera is used when rendering the scene.

        const glm::mat4& getViewMatrix() const { return mView; };
        const glm::vec3& getPosition() const { return mPosition; };
        const glm::vec3& getForwardDirection() const { return mFront; };

        void DrawImGui() {};
        bool operator== (const Camera& pOther) const
        {
            return mPosition == pOther.mPosition &&
                mYaw == pOther.mYaw &&
                mPitch == pOther.mPitch &&
                mFront == pOther.mFront &&
                mUp == pOther.mUp &&
                mRight == pOther.mRight &&
                mView == pOther.mView &&
                mMovementSpeed == pOther.mMovementSpeed &&
                mMouseSensitivity == pOther.mMouseSensitivity &&
                mPrimaryCamera == pOther.mPrimaryCamera &&
                mZoom == pOther.mZoom;
        };
        bool operator != (const Camera& pOther) const { return !(*this == pOther); }

    private:
        // Calculates mFront, mRight, mUp and mView members from the Camera's Euler angles mYaw and mPitch.
        void updateCameraVectors();

        glm::vec3 mPosition; // Position of the camera.
        float mYaw;
        float mPitch;

        glm::vec3 mFront; // Normalised direction the camera is facing.
        glm::vec3 mUp;    // Normalised camera local up direction.
        glm::vec3 mRight; // Normalised camera local right direction.
        glm::mat4 mView;  // View transformation matrix.

        float mMovementSpeed;
        float mMouseSensitivity;
        float mZoom;

        static inline const glm::vec3 WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    };
} // namespace Component