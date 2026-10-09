#include "FreeCameraController.h"
#include "../Input/Input.h"

#include <cmath>

namespace Sei::FreeCameraController
{
    void Update(Camera& camera, float deltaTime, float speed)
    {
        if (Input::IsMouseCaptured())
        {
            const float sensitivity = Input::mouseSensitivity;
            camera.Rotate(Input::GetMouseDeltaX() * sensitivity,
                          -Input::GetMouseDeltaY() * sensitivity);
        }

        const float forwardInput = static_cast<float>(Input::KeyDown('W')) -
                                   static_cast<float>(Input::KeyDown('S'));
        const float rightInput = static_cast<float>(Input::KeyDown('D')) -
                                 static_cast<float>(Input::KeyDown('A'));

        // Normalize so diagonal movement isn't faster.
        const float length = std::sqrt(forwardInput * forwardInput + rightInput * rightInput);
        if (length == 0.0f)
            return;

        // Horizontal directions for walking.
        auto forward = camera.GetForward();
        const float horizontalLength = std::sqrt(forward.x * forward.x + forward.z * forward.z);
        forward.x /= horizontalLength;
        forward.z /= horizontalLength;
        const auto right = camera.GetRight();
        const float distance = speed * deltaTime / length;
        auto position = camera.GetPosition();
        position.x += (forward.x * forwardInput + right.x * rightInput) * distance;
        position.z += (forward.z * forwardInput + right.z * rightInput) * distance;
        camera.SetPosition(position.x, position.y, position.z);
    }
}
