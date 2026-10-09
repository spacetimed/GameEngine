#include "Player.h"
#include "../../Sei/Input/Input.h"
#include "../../Sei/Window/Window.h"

#include <cmath>

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
		Sei::Camera camera;
	}

	void spawn(float x, float y, float z)
	{
		position = { x, y, z };
		velocity = {};
		camera.SetPosition(x, y + eyeHeight, z);
		camera.SetRotation(0.0f, 0.0f);

		RECT client = {};
		GetClientRect(Sei::Window::GetHandle(), &client);
		const float aspect = static_cast<float>(client.right - client.left) /
			static_cast<float>(client.bottom - client.top);
		const float horizontalFov = DirectX::XMConvertToRadians(103.0f);
		const float verticalFov = 2.0f * std::atan(std::tan(horizontalFov * 0.5f) / aspect);
		camera.SetPerspective(verticalFov, aspect, 0.1f, 100.0f);
	}

	void update(float deltaTime)
	{
		if (Sei::Input::IsMouseCaptured())
			camera.Rotate(Sei::Input::GetMouseDeltaX() * Sei::Input::mouseSensitivity,
				-Sei::Input::GetMouseDeltaY() * Sei::Input::mouseSensitivity);

		const float forward = static_cast<float>(Sei::Input::KeyDown('W')) -
			static_cast<float>(Sei::Input::KeyDown('S'));
		const float right = static_cast<float>(Sei::Input::KeyDown('D')) -
			static_cast<float>(Sei::Input::KeyDown('A'));
		move(forward, right, deltaTime);
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
		position.x += velocity.x * deltaTime;
		position.z += velocity.z * deltaTime;
		camera.SetPosition(position.x, position.y + eyeHeight, position.z);
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
