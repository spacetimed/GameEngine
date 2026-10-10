#include "View.h"

namespace Game::Player::View
{
    void Toggle()
    {
        isThirdPerson = !isThirdPerson;
        mode = isThirdPerson ? "3rd" : "1st";
    }

    void Update(Sei::Camera& camera, const DirectX::XMFLOAT3& feet, float eyeHeight)
    {
        const float distance = isThirdPerson ? 4.0f : 0.0f;
        const auto forward = camera.GetForward();
        camera.SetPosition(
            feet.x - forward.x * distance,
            feet.y + eyeHeight - forward.y * distance,
            feet.z - forward.z * distance);
    }
}
