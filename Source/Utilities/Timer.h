#pragma once
#include <cstdint>
#include <chrono>

namespace Dive
{
	class Engine;

	class Time
	{
		friend Engine;

	public:
		static double GetElapsedTime() { return s_elapsedTime; }
		static float GetDeltaTime() { return s_deltaTime; }
		
		uint16_t GetFps() const { return s_fps; }

	private:
		Time();
		~Time() = default;

		static void tick();

	private:
		static std::chrono::steady_clock::time_point s_lastTickTime;
		static float s_deltaTime;
		static double s_elapsedTime;
		static uint16_t s_fps;
	};
}