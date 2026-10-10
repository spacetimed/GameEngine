#pragma once

#include <DirectXMath.h>

#include "../../Sei/Camera/Camera.h"
#include "../../Sei/Render/Render.h"
#include "View.h"

namespace Game::Player
{
	bool Initialize(Sei::Render::RenderQueue& renderQueue, const DirectX::XMFLOAT3& spawnPosition);
	void Submit();
	void Shutdown();
	void Update(float deltaTime);
	void Move(float forward, float right, float deltaTime);
	DirectX::XMFLOAT3 GetPosition();
	Sei::Camera& GetCamera();
}
