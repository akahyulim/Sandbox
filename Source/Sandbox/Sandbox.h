#pragma once
#include <memory>
#include <array>
#include <imgui/imgui.h>

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

	enum class eMaterials
	{
		Default,
		Tiles,
		Metal_Plate,
		Rusty_Metal_Grid,
		Stacked_Brick_Wall,
		Marble_Cliff,
		Rust_Metal,
		Count
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
		void renderLightIcons(const ImVec2& viewportPos, const ImVec2& viewportSize);
		void drawCanvasContextMenu();
		void handleSceneClick(const ImVec2& viewportPos, const ImVec2& viewportSize);
		void renderGizmo(const ImVec2& viewportPos, const ImVec2& viewportSize);
		void showEnviroment();
		void showInspector();
		void showQuit();

		void setSelectedObject(GameObject* gameObject);

		void loadResources();

		void sceneEmpty();
		void sceneInnocent();
		void sceneLighting();

	private:
		std::unique_ptr<Engine> m_engine;
		std::unique_ptr<ImGuiManager> m_gui;

		Scene* m_scene = nullptr;

		GameObject* m_mainCamera = nullptr;
		GameObject* m_directionalLight = nullptr;
		GameObject* m_selectedObject = nullptr;
		GameObject* m_contextTargetObject = nullptr;
		//GameObject* m_field = nullptr;

		bool m_showEnviromentMenu = false;
		bool m_showInspectorMenu = false;
		bool m_showQuitMenu = false;

		bool m_isSceneViewHovered = false;

		std::unordered_map<std::string, TextureHandle> m_skyCubemaps;

		TextureHandle m_pointLightIcon = INVALID_TEXTURE_HANDLE;
		TextureHandle m_spotLightIcon = INVALID_TEXTURE_HANDLE;
		
		// ================================================================
		
		float m_cameraPitch = 0.0f;
		float m_cameraYaw = 0.0f;
	};
}