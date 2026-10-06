#pragma once

#include "Camera.h"

namespace Sei::FreeCameraController
{
    void Update(Camera& camera, float deltaTime, float speed = 5.0f);
}
