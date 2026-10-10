#pragma once

#include <DirectXMath.h>

#include "../../Sei/Camera/Camera.h"

namespace Game::Player
{
	bool spawn(float x, float y, float z);
	void draw();
	void clear();
	void update(float deltaTime);
	void move(float forward, float right, float deltaTime);
	DirectX::XMFLOAT3 getPosition();
	Sei::Camera& getCamera();
}
