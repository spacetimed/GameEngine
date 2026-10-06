#pragma once

#include <DirectXMath.h>

namespace Sei
{
    class Camera
    {
    public:
        void SetPosition(float x, float y, float z);
        DirectX::XMFLOAT3 GetPosition() const;
        void LookAt(float x, float y, float z);

        // Angles are in radians.
        void SetRotation(float yaw, float pitch);
        void Rotate(float yawDelta, float pitchDelta);

        void SetPerspective(
            float verticalFov,
            float aspectRatio,
            float nearClip,
            float farClip);

        DirectX::XMFLOAT3 GetForward() const;
        DirectX::XMFLOAT3 GetRight() const;

        DirectX::XMMATRIX GetViewMatrix() const;
        DirectX::XMMATRIX GetProjectionMatrix() const;

    private:
        DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };

        // Zero rotation faces +Z; +Y is up.
        float yaw = 0.0f;
        float pitch = 0.0f;

        float verticalFov = DirectX::XM_PIDIV4;
        float aspectRatio = 1.0f;
        float nearClip = 0.1f;
        float farClip = 100.0f;
    };
}
