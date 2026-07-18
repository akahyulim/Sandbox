#include "pch.h"
#include "Timer.h"

namespace Dive
{
	Timer::Timer()
		: m_elapsedTimeMS(0)
		, m_deltaTimeMS(0)
		, m_fps(0)
	{
	}

	Timer::~Timer() = default;

	void Timer::Start()
	{
		m_lastTickTime = std::chrono::steady_clock::now();
	}

	void Timer::Tick()
	{
		auto currentTickTime = std::chrono::steady_clock::now();
		m_deltaTimeMS = std::chrono::duration<float, std::milli>(currentTickTime - m_lastTickTime).count();
		m_elapsedTimeMS += m_deltaTimeMS;
		m_lastTickTime = currentTickTime;

		static double lastTimeMS = 0;
		static uint16_t frameCount = 0;
		frameCount++;
		if (m_elapsedTimeMS - lastTimeMS >= 1000.0)
		{
			m_fps = frameCount;
			frameCount = 0;
			lastTimeMS = m_elapsedTimeMS;
		}
	}
}