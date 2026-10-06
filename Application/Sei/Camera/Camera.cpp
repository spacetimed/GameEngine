
#include "Camera.h"

#include <algorithm>
#include <cmath>

namespace Sei
{
    void Camera::SetPosition(float x, float y, float z)
    {
        position = { x, y, z };
    }

    DirectX::XMFLOAT3 Camera::GetPosition() const
    {
        return position;
    }

    void Camera::SetRotation(float newYaw, float newPitch)
    {
        yaw = newYaw;

        // Prevent looking exactly straight up/down.
        const float limit = DirectX::XM_PIDIV2 - 0.01f;
        pitch = std::clamp(newPitch, -limit, limit);
    }

    void Camera::LookAt(float x, float y, float z)
    {
        const float dx = x - position.x;
        const float dy = y - position.y;
        const float dz = z - position.z;
        if (dx == 0.0f && dy == 0.0f && dz == 0.0f)
            return;
        SetRotation(std::atan2(dx, dz), std::atan2(dy, std::sqrt(dx * dx + dz * dz)));
    }

    void Camera::Rotate(float yawDelta, float pitchDelta)
    {
        SetRotation(yaw + yawDelta, pitch + pitchDelta);
    }

    void Camera::SetPerspective(
        float newVerticalFov,
        float newAspectRatio,
        float newNearClip,
        float newFarClip)
    {
        verticalFov = newVerticalFov;
        aspectRatio = newAspectRatio;
        nearClip = newNearClip;
        farClip = newFarClip;
    }

    DirectX::XMFLOAT3 Camera::GetForward() const
    {
        // Positive yaw turns toward +X; positive pitch looks up.
        return {
            std::sin(yaw) * std::cos(pitch),
            std::sin(pitch),
            std::cos(yaw) * std::cos(pitch)
        };
    }

    DirectX::XMFLOAT3 Camera::GetRight() const
    {
        return {
            std::cos(yaw),
            0.0f,
            -std::sin(yaw)
        };
    }

    DirectX::XMMATRIX Camera::GetViewMatrix() const
    {
        const DirectX::XMFLOAT3 forward = GetForward();

        return DirectX::XMMatrixLookToLH(
            DirectX::XMLoadFloat3(&position),
            DirectX::XMLoadFloat3(&forward),
            DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
    }

    DirectX::XMMATRIX Camera::GetProjectionMatrix() const
    {
        return DirectX::XMMatrixPerspectiveFovLH(
            verticalFov,
            aspectRatio,
            nearClip,
            farClip);
    }
}
