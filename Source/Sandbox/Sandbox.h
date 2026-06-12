#pragma once
#include <memory>

namespace Dive
{
	struct DirectionalLightEditor
	{
		DirectX::XMFLOAT3 color = { 1.0f, 1.0f, 1.0f };
		float intensity = 1.0f;

		// 🌟 유저가 슬라이더로 조절할 회전 각도 (도 단위)
		float pitch = 45.0f; // 상하 각도
		float yaw = 45.0f;   // 좌우 각도
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

		bool m_showLightDialog = false;

		DirectionalLightEditor m_lightEditorData;
	};
}