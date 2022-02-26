#pragma once

#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"

#include "functional"

// Camera operating using Euler angles to calculate the corresponding Vectors and Matrices to define a view in 3D space.
// Constructor allows subscribing to view change events, passing the new glm::mat4 view matrix on change.
class Camera
{
public:
    Camera(const Camera&) = delete;
    Camera(const glm::vec3& pPosition
    , const std::function<void(const glm::mat4&)>& pOnViewChangeCallback
    , const std::function<void(const glm::vec3&)>& pOnViewPositionChangeCallback
    , const float& pYaw = -90.0f
    , const float& pPitch = 0.0f);

    enum MoveDirection{ Forward, Backward, Left, Right, Up, Down };
    void move(const MoveDirection& pDirection); // Process key evenets to move the Camera mPosition.
    void ProcessMouseMove(const float& pXOffset, const float& pYOffset, const bool& pConstrainPitch = true); // Processes mouse movement to allow moving camera in it's current mPosition.
    void processScroll(const float& pOffset); // Process mouse scrollwheel events. Applies a zoom on the camera.

private:
    // Calculates mFront, mRight, mUp and mView members from the Camera's Euler angles mYaw and mPitch.
    void updateCameraVectors();

    glm::vec3 mPosition; // Position of the camera.
    float mYaw;
    float mPitch;

    glm::vec3 mFront;   // Normalised direction the camera is facing.
    glm::vec3 mUp;      // Normalised camera local up direction.
    glm::vec3 mRight;   // Normalised camera local right direction.
    glm::mat4 mView;    // View transformation matrix.

    std::function<void(const glm::mat4&)> mOnViewChange; // Called when mView changes.
    std::function<void(const glm::vec3&)> mOnViewPositionChange; // Called when mView changes.

    float mMovementSpeed;
    float mMouseSensitivity;
    float mZoom;

    static inline const glm::vec3 WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
};