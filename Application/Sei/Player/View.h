#pragma once

#include "../Camera/Camera.h"
#include <string>

namespace Sei::Player::View
{
	inline bool isThirdPerson = false;
	inline std::string mode = "first person";

	void toggle();
	void update(Camera& camera, const DirectX::XMFLOAT3& feet, float eyeHeight);
}
