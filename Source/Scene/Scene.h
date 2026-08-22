#pragma once
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <filesystem>

#include "Core/Types.h"
#include "Resource/Preset.h"
#include "Components/Component.h"
//#include "GameObject.h"
#include "Rendering/TextureManager.h"

namespace Dive
{
	// 이유는 모르겠지만 전방선언만으로 해결되지 않는다.
	class GameObject;

	class Camera;
	class Light;
	class MeshRenderer;

	struct EnviromentData
	{
		TextureHandle skyboxCubemap = INVALID_TEXTURE_HANDLE;
	};

	class Scene
	{
	public:
		Scene();
		~Scene();

		void Clear();

		void Update();
		
		GameObject* CreateGameObject();
		GameObject* CreateGameObjectFromID(uint64_t id);
		void RemoveGameObject(GameObject* gameObject);
		void RemoveGameObjectByID(uint64_t id);

		GameObject* AddPresetObject(ePresetType type);
		GameObject* AddModelObject(const std::filesystem::path& modelPath);

		void GetRootGameObjects(std::vector<GameObject*>& outRoots);

		GameObject* GetCamera() const { return m_camera; }
		GameObject* GetDirectionalLight() const { return m_directionalLight; }
		const std::vector<GameObject*>& GetLightQueue() const { return m_lights; }
		const std::vector<GameObject*>& GetRenderables() const { return m_renderables; }

		bool SaveToFile(const std::filesystem::path& filepath);
		bool LoadFromFile(const std::filesystem::path& filepath);

		bool IsDirty() const { return m_isDirty; }
		void SetDirty() { m_isDirty = true; }
		void ClearDirty() { m_isDirty = false; }

		std::string GetName() const { return m_name; }
		void SetName(const std::string& name) { m_name = name; }

		EnviromentData& GetEnviroment() { return m_enviroment; }

	private:
		std::string m_name = "Playgorund";
			
		std::vector<std::unique_ptr<GameObject>> m_pendingAdditions;
		std::unordered_map<uint64_t, std::unique_ptr<GameObject>> m_gameObjectMap;
		std::vector<GameObject*> m_pendingRemovals;
		
		GameObject* m_camera = nullptr;
		GameObject* m_directionalLight = nullptr;
		std::vector<GameObject*> m_lights;
		std::vector<GameObject*> m_renderables;

		bool m_isDirty = false;

		EnviromentData m_enviroment;
	};
}