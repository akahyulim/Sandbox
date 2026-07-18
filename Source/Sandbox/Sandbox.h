#pragma once
#include <memory>

#include "Core/Types.h"
#include "Core/Engine.h"
#include "ImGuiManager.h"

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
	class GameObject;
	class Scene;
	struct WindowEventData;

	struct SandboxInit
	{
		EngineInit engin_init;
	};

	class Sandbox
	{
	public:
		Sandbox(const SandboxInit& init);
		~Sandbox();

		bool Initialize();
		void Run();

		void OnWindowEvent(const WindowEventData& data);

	private:
		void cameraControll(float dt);
		
		// 각종 view는 메서드화

	private:
		std::unique_ptr<Engine> m_engine;
		std::unique_ptr<ImGuiManager> m_gui;
		
		// ================================================================
		
		GameObject* m_mainCamera = nullptr;
		GameObject* m_directionalLight = nullptr;
		GameObject* m_selected = nullptr;

		float m_cameraPitch = 0.0f;
		float m_cameraYaw = 0.0f;

		bool m_showEnvDiralog = false;

		EnviromentData m_enviromentData;
	};
}