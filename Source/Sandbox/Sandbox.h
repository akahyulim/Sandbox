#pragma once
#include <memory>
#include <array>

#include "Core/Types.h"
#include "Core/Engine.h"
#include "ImGuiManager.h"

namespace Dive
{
	// 이걸 Scene의 EnviromentData에 흡수?
	/*
	struct EnviromentData
	{
		Color skyColor;
		Color groundColor;

		// 여전히 좀 아쉽다. LightData에 intensity를 포함하는 쪽도 생각해보자.
		Color lightColor = Color{ 1.0f, 1.0f, 1.0f, 1.0f };	// color = r, g, b, intensity = a

		float lightPitch = 45.0f; // 상하 각도
		float lightYaw = 45.0f;   // 좌우 각도
	};
	*/
	class GameObject;
	class Scene;
	struct WindowEventData;

	struct SandboxInit
	{
		EngineInit engin_init;
	};

	class Sandbox
	{
		enum
		{
			Flag_Enviroment,
			Flag_Hierarchy,
			Flag_Count
		};

	public:
		Sandbox(const SandboxInit& init);
		~Sandbox();

		void Shutdown();

		void Run();

		void OnWindowEvent(const WindowEventData& data);

	private:
		void cameraControll();
		
		void scene();
		void menu();
		void enviroment();
		void hierarchy();

		void newScene();

	private:
		std::unique_ptr<Engine> m_engine;
		std::unique_ptr<ImGuiManager> m_gui;

		Scene* m_scene = nullptr;

		GameObject* m_mainCamera = nullptr;
		GameObject* m_directionalLight = nullptr;
		GameObject* m_selectedObject = nullptr;

		std::array<bool, Flag_Count> m_windowFlags = { false };
		
		// ================================================================
		
		float m_cameraPitch = 0.0f;
		float m_cameraYaw = 0.0f;

		bool m_showEnvDiralog = false;

		//EnviromentData m_enviromentData;
	};
}