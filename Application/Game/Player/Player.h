#pragma once

#include <DirectXMath.h>

#include "../../Sei/Camera/Camera.h"

namespace Game::Player
{
	void spawn(float x, float y, float z);
	void update(float deltaTime);
	void move(float forward, float right, float deltaTime);
	DirectX::XMFLOAT3 getPosition();
	Sei::Camera& getCamera();
}
