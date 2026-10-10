#include "Player.h"
#include "../World/World.h"
#include "../../Sei/Input/Input.h"
#include "../../Sei/Window/Window.h"
#include "../../Sei/AssetLoader/SceneDataLoader.h"
#include "../../Sei/Render/Render.h"
#include "../../Sei/Collision/View.h"

#include <cmath>
#include <iostream>
#include <algorithm>
#include <cfloat>

namespace Game::Player
{

	// private player state information
	namespace
	{
		float moveSpeed = 30.0f;
		float gravity = 20.0f;
		float jumpHeight = 1.0f;
		float eyeHeight = 1.7f;
		DirectX::XMFLOAT3 position = {}; // Feet position.
		DirectX::XMFLOAT3 velocity = {};
		bool grounded = false;
		Sei::Camera camera;
		Sei::SceneDataLoader::SceneObject ModelObject;
		Sei::Shader modelShader;

		Sei::Player::Collision::AABB collisionBox;
	}

	bool spawn(float x, float y, float z)
	{
		std::string error;

		position = { x, y, z };
		velocity = {};
		grounded = false;
		camera.SetRotation(0.0f, 0.0f);
		Sei::Player::View::update(camera, position, eyeHeight);

		// Load once; respawning reuses the same GPU mesh and shader.
		if (!ModelObject.mesh.vertexBuffer)
		{
			Sei::SceneDataLoader::SceneData ModelData;
			if (!Sei::SceneDataLoader::loadSceneData("Resources/Models/player.glb", ModelData, error) ||
				!Sei::Render::CreateShader("Resources/Shaders/Solid.hlsl", modelShader, error))
			{
				std::cerr << "could not load player model: " << error << '\n';
				return false;
			}
			const auto model = ModelData.objects.find("Player");
			if (model == ModelData.objects.end())
			{
				std::cerr << "player.glb contains no object named Player\n";
				return false;
			}
			ModelObject = model->second; // object name in Blender: Player
		}

		// detect collisionBox here
		const auto& min = ModelObject.mesh.boundsMin;
		const auto& max = ModelObject.mesh.boundsMax;
		collisionBox.min = { FLT_MAX, FLT_MAX, FLT_MAX };
		collisionBox.max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
		for (int corner = 0; corner < 8; ++corner)
		{
			const auto point = DirectX::XMVectorSet(corner & 1 ? max.x : min.x,
				corner & 2 ? max.y : min.y, corner & 4 ? max.z : min.z, 1.0f);
			DirectX::XMFLOAT3 p;
			DirectX::XMStoreFloat3(&p, DirectX::XMVector3TransformCoord(point,
				DirectX::XMLoadFloat4x4(&ModelObject.transform)));
			collisionBox.min.x = (std::min)(collisionBox.min.x, p.x);
			collisionBox.min.y = (std::min)(collisionBox.min.y, p.y);
			collisionBox.min.z = (std::min)(collisionBox.min.z, p.z);
			collisionBox.max.x = (std::max)(collisionBox.max.x, p.x);
			collisionBox.max.y = (std::max)(collisionBox.max.y, p.y);
			collisionBox.max.z = (std::max)(collisionBox.max.z, p.z);
		}

		RECT client = {};
		GetClientRect(Sei::Window::GetHandle(), &client);
		const float aspect = static_cast<float>(client.right - client.left) /
			static_cast<float>(client.bottom - client.top);
		const float horizontalFov = DirectX::XMConvertToRadians(103.0f);
		const float verticalFov = 2.0f * std::atan(std::tan(horizontalFov * 0.5f) / aspect);
		camera.SetPerspective(verticalFov, aspect, 0.1f, 100.0f);
		return true;
	}

	void draw()
	{
		if (!Sei::Player::View::isThirdPerson || !ModelObject.mesh.vertexBuffer) return;
		const auto forward = camera.GetForward();
		const float yaw = std::atan2(forward.x, forward.z);
		DirectX::XMFLOAT4X4 transform;
		DirectX::XMStoreFloat4x4(&transform, DirectX::XMLoadFloat4x4(&ModelObject.transform) *
			DirectX::XMMatrixRotationY(yaw) *
			DirectX::XMMatrixTranslation(position.x, position.y, position.z));
		Sei::Render::Draw(ModelObject.mesh, modelShader, camera, transform);
	
		// draw collision box here
		Sei::Render::DrawBox(
			{ position.x + collisionBox.min.x, position.y + collisionBox.min.y, position.z + collisionBox.min.z },
			{ position.x + collisionBox.max.x, position.y + collisionBox.max.y, position.z + collisionBox.max.z }, camera);
	}

	void clear()
	{
		ModelObject = {};
		modelShader = {};
	}

	void update(float deltaTime)
	{
		if (grounded && Sei::Input::KeyPressed(VK_SPACE))
		{
			velocity.y = std::sqrt(2.0f * gravity * jumpHeight);
			grounded = false;
		}

		if (Sei::Input::IsMouseCaptured())
			camera.Rotate(Sei::Input::GetMouseDeltaX() * Sei::Input::mouseSensitivity,
				-Sei::Input::GetMouseDeltaY() * Sei::Input::mouseSensitivity);

		const float forward = static_cast<float>(Sei::Input::KeyDown('W')) -
			static_cast<float>(Sei::Input::KeyDown('S'));
		const float right = static_cast<float>(Sei::Input::KeyDown('D')) -
			static_cast<float>(Sei::Input::KeyDown('A'));
		move(forward, right, deltaTime);
		Game::World::checkOverlaps({
			{ position.x + collisionBox.min.x, position.y + collisionBox.min.y, position.z + collisionBox.min.z },
			{ position.x + collisionBox.max.x, position.y + collisionBox.max.y, position.z + collisionBox.max.z }
		});
	}

	void move(float forwardInput, float rightInput, float deltaTime)
	{
		velocity.x = velocity.z = 0.0f;
		const float length = std::sqrt(forwardInput * forwardInput + rightInput * rightInput);
		if (length > 0.0f)
		{
			// Horizontal walking, with equal speed on diagonals.
			auto forward = camera.GetForward();
			const float horizontalLength = std::sqrt(forward.x * forward.x + forward.z * forward.z);
			forward.x /= horizontalLength;
			forward.z /= horizontalLength;
			const auto right = camera.GetRight();
			velocity.x = (forward.x * forwardInput + right.x * rightInput) * moveSpeed / length;
			velocity.z = (forward.z * forwardInput + right.z * rightInput) * moveSpeed / length;
		}
		// Sweep the entire movement so fast frames can't skip through thin objects.
		auto tryMove = [&](const DirectX::XMFLOAT3& delta)
		{
			const Sei::Player::Collision::AABB box = {
				{ position.x + collisionBox.min.x, position.y + collisionBox.min.y, position.z + collisionBox.min.z },
				{ position.x + collisionBox.max.x, position.y + collisionBox.max.y, position.z + collisionBox.max.z }
			};
			const auto hit = Game::World::sweep(box, delta);
			float fraction = hit.fraction;
			if (hit.hit)
			{
				const float distance = std::sqrt(delta.x*delta.x + delta.y*delta.y + delta.z*delta.z);
				if (distance > 0.0f) fraction = (std::max)(0.0f, fraction - 0.001f / distance);
			}
			position.x += delta.x * fraction;
			position.y += delta.y * fraction;
			position.z += delta.z * fraction;
			return hit;
		};

		// Gravity is applied every frame; collision stops downward motion at the floor.
		velocity.y -= gravity * deltaTime;
		const auto verticalHit = tryMove({ 0.0f, velocity.y * deltaTime, 0.0f });
		if (verticalHit.hit) velocity.y = 0.0f;
		if (tryMove({ velocity.x * deltaTime, 0.0f, velocity.z * deltaTime }).hit)
			velocity.x = velocity.z = 0.0f;

		// Check support at the final position, including after walking off an edge.
		const Sei::Player::Collision::AABB box = {
			{ position.x + collisionBox.min.x, position.y + collisionBox.min.y, position.z + collisionBox.min.z },
			{ position.x + collisionBox.max.x, position.y + collisionBox.max.y, position.z + collisionBox.max.z }
		};
		const auto support = Game::World::sweep(box, { 0.0f, -0.002f, 0.0f });
		grounded = velocity.y <= 0.0f && support.hit && support.normal.y > 0.5f;
		Sei::Player::View::update(camera, position, eyeHeight);
	}
	DirectX::XMFLOAT3 getPosition()
	{
		return position;
	}

	Sei::Camera& getCamera()
	{
		return camera;
	}
}
