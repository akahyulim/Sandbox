#include "pch.h"
#include "Timer.h"

namespace Dive
{
	std::chrono::steady_clock::time_point Time::s_lastTickTime;
	float Time::s_deltaTime = 0.0f;
	double Time::s_elapsedTime = 0;
	uint16_t Time::s_fps = 0;

	Time::Time()
	{
		s_lastTickTime = std::chrono::steady_clock::now();
	}

	void Time::tick()
	{
		auto currentTickTime = std::chrono::steady_clock::now();
		s_deltaTime = std::chrono::duration<float, std::milli>(currentTickTime - s_lastTickTime).count();
		s_elapsedTime += s_deltaTime;
		s_lastTickTime = currentTickTime;

		static double lastTime = 0;
		static uint16_t frameCount = 0;
		frameCount++;
		if (s_elapsedTime - lastTime >= 1000.0)
		{
			s_fps = frameCount;
			frameCount = 0;
			lastTime = s_elapsedTime;
		}
	}
}