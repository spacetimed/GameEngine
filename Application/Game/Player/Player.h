#pragma once

#include <DirectXMath.h>

#include "../../Sei/Camera/Camera.h"

namespace Game::Player
{
	float moveSpeed = 20.0f;
	float gravity = 20.0f;
	float jumpHeight = 1.0f;
	float eyeHeight = 1.7f;

	DirectX::XMFLOAT3 position = {}; // current position

	Sei::Camera camera;

	void spawn(float x, float y, float z);
	void move(int direction); // updates position
}