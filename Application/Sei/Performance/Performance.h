#pragma once

#include <string>
#include <chrono>
#include <format>

namespace Sei::Performance
{
	extern std::string currFps;
	extern std::string currFrametime;
	extern std::string resolution;

	void Init(int resW, int resH);
	void tick();
}