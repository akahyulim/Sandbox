#pragma once
#include <memory>
#include <imgui/imgui.h>

#include "Core/Types.h"
#include "Core/Engine.h"
#include "ImGuiManager.h"
#include "Rendering/TextureManager.h"

namespace Dive
{
	struct WindowEventData;
	class Scene;
	class GameObject;
	class Light;
	class Mesh;
	class Material;
	
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

	enum class eInspectorMode
	{
		ShowComponents,
		ShowMaterial,
		NewMaterial,
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
		Sandbox() = delete;

		void loadResources();

		// Scenes	=> 직렬화 구현 후 제거
		void sceneEmpty();
		void sceneInnocent();
		void sceneLighting();

		void cameraControll();
	
		// SceneView
		void sceneView();
		void renderLightIcons(const ImVec2& viewportPos, const ImVec2& viewportSize);
		void drawCanvasContextMenu();
		void handleSceneClick(const ImVec2& viewportPos, const ImVec2& viewportSize);
		void renderGizmo(const ImVec2& viewportPos, const ImVec2& viewportSize);
		
		// Menus
		void showEnvironment();
		void showInspector();
		void showOption();

		// Insepctor
		void showComponents();
		void showMesh();
		void showMaterial();
		void showNewMaterial();

		// Popups
		void popupMeshes();
		void popupMaterials();

		// Add Menu
		GameObject* createPointLight();
		GameObject* createSpotLight();

		void setSelectedObject(GameObject* gameObject);

	private:
		std::unique_ptr<Engine> m_engine;
		std::unique_ptr<ImGuiManager> m_gui;

		Scene* m_scene = nullptr;

		// Objects
		GameObject* m_mainCamera = nullptr;
		GameObject* m_directionalLight = nullptr;
		GameObject* m_selectedObject = nullptr;
		GameObject* m_contextTargetObject = nullptr;

		Mesh* m_selectedMesh = nullptr;
		Material* m_selectedMaterial = nullptr;

		bool m_isSceneViewHovered = false;

		// Menus
		bool m_showEnvironmentMenu = false;
		bool m_showInspectorMenu = false;
		bool m_showOptionMenu = false;

		std::unordered_map<std::string, TextureHandle> m_skyCubemaps;

		// Icons
		TextureHandle m_pointLightIcon = INVALID_TEXTURE_HANDLE;
		TextureHandle m_spotLightIcon = INVALID_TEXTURE_HANDLE;
		
		float m_cameraPitch = 0.0f;
		float m_cameraYaw = 0.0f;

		eInspectorMode m_inspectorMode = eInspectorMode::ShowComponents;
	};
}