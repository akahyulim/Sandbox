#pragma once
#include <cstdint>
#include <chrono>

namespace Dive
{
	class Timer
	{
	public:
		Timer();
		~Timer();

		void Start();

		void Tick();

		double GetElapsedTimeMS() const { return m_elapsedTimeMS; }
		double GetElapsedTimeSec() const { return m_elapsedTimeMS / 1000.0; }

		float GetDeltaTimeMS() const { return m_deltaTimeMS; }
		float GetDeltaTimeSec() const { return m_deltaTimeMS / 1000.0f; }

		uint16_t GetFps() const { return m_fps; }

	private:
		double m_elapsedTimeMS;
		float m_deltaTimeMS;
		std::chrono::steady_clock::time_point m_lastTickTime;
		uint16_t m_fps;
	};
}