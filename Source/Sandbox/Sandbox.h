#pragma once
#include <memory>

#include "Core/Types.h"

namespace Dive
{
	struct EnviromentData
	{
		Color skyColor;
		Color groundColor;

		// 여전히 좀 아쉽다. cbLight에 intensity를 포함하는 쪽도 생각해보자.
		Color lightColor = Color{ 1.0f, 1.0f, 1.0f, 1.0f };	// color = r, g, b, intensity = a

		float lightPitch = 45.0f; // 상하 각도
		float lightYaw = 45.0f;   // 좌우 각도
	};

	class Timer;
	class Graphics;
	class Renderer;
	class Scene;

	class Sandbox
	{
	public:
		Sandbox();
		~Sandbox();

		bool Initialize();
		void Run();

	private:
		void cameraControll(float dt);

	private:
		std::unique_ptr<Timer> m_timer;
		std::unique_ptr<Graphics> m_graphics;
		std::unique_ptr<Renderer> m_renderer;

		std::unique_ptr<Scene> m_scene;

		float m_cameraPitch = 0.0f;
		float m_cameraYaw = 0.0f;

		bool m_showEnvDiralog = false;

		EnviromentData m_enviromentData;
	};
}