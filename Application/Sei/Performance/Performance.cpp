/*
	provides and updates:
		- resolution
		- fps
		- frametime
*/

#include "Performance.h"

namespace Sei::Performance
{
	namespace
	{
		std::chrono::steady_clock::time_point previousTime;
		float fpsElapsed = 0.0f;
		unsigned int fpsFrames = 0;
	}

	// bookkepeping, can access ref's from parent assuming .tick() is used throughout renders
	// public
	std::string currFps;
	std::string currFrametime;
	std::string resolution;
	float deltaTime;

	void Initialize(int resW, int resH)
	{
		previousTime = std::chrono::steady_clock::now();
		fpsElapsed = 0.0f;
		fpsFrames = 0;
		deltaTime = 0.0f;
		currFps = "0";
		currFrametime = "0.00 ms";
		resolution = std::format("{}x{}", resW, resH);
	}

	void tick()
	{
		const auto now = std::chrono::steady_clock::now();
		deltaTime = std::chrono::duration<float>(now - previousTime).count();
		previousTime = now;
		fpsElapsed += deltaTime;
		++fpsFrames;
		if (fpsElapsed >= 0.5f)
		{
			currFps = std::to_string(static_cast<unsigned int>(fpsFrames / fpsElapsed + 0.5f));
			currFrametime = std::format("{:.2f} ms", fpsElapsed * 1000.0f / fpsFrames);
			fpsElapsed = 0.0f;
			fpsFrames = 0;
		}
	}
}
