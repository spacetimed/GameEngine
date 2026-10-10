#pragma once

#include <string>
#include <chrono>
#include <format>

namespace Sei::Performance
{
	extern std::string currFps;
	extern std::string currFrametime;
	extern std::string resolution;
	extern float deltaTime;

	void Initialize(int resW, int resH);
	void tick();
}