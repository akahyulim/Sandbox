#pragma once
#include <memory>
#include <array>

#include "Core/Types.h"
#include "Core/Engine.h"
#include "ImGuiManager.h"
#include "Rendering/TextureManager.h"

namespace Dive
{
	class ImGuiManager;
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
		~Sandbox() = default;

		void Run();
		void Shutdown();

		void OnWindowEvent(const WindowEventData& data);

	private:
		void cameraControll();
		
		void sceneView();

		void drawCanvasContextMenu();
		
		void showEnviroment();
		void showInspector();
		void showQuit();

		void newScene();
		void setSelectedObject(GameObject* gameObject);

	private:
		std::unique_ptr<Engine> m_engine;
		std::unique_ptr<ImGuiManager> m_gui;

		Scene* m_scene = nullptr;

		GameObject* m_mainCamera = nullptr;
		GameObject* m_directionalLight = nullptr;	// 역시 현재 사용하지 않고 있다.
		GameObject* m_selectedObject = nullptr;
		GameObject* m_contextTargetObject = nullptr;
		GameObject* m_field = nullptr;

		bool m_showEnviromentMenu = false;
		bool m_showInspectorMenu = false;
		bool m_showQuitMenu = false;

		bool m_isSceneViewHovered = false;

		std::unordered_map<std::string, TextureHandle> m_skyCubemaps;
		
		// ================================================================
		
		float m_cameraPitch = 0.0f;
		float m_cameraYaw = 0.0f;
	};
}