#pragma once

#include "../../Sei/Camera/Camera.h"
#include <string>

namespace Game::Player::View
{
	inline bool isThirdPerson = false;
	inline std::string mode = "first person";

	void Toggle();
	void Update(Sei::Camera& camera, const DirectX::XMFLOAT3& feet, float eyeHeight);
}
