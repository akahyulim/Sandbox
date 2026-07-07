#pragma once
#include <string>
#include <memory>
#include <vector>
#include <filesystem>

#include "Core/Types.h"
#include "Resource/Preset.h"

#include "GameObject.h"

namespace Dive
{
	// 이유는 모르겠지만 전방선언만으로 해결되지 않는다.
	//class GameObject;

	class Camera;
	class Light;
	class MeshRenderer;

	class Scene
	{
	public:
		Scene();
		~Scene();

		void Update(float dt);
		
		GameObject* CreateGameObject(uint64_t id = AUTO_ID);

		GameObject* AddPresetObject(ePresetType type);
		GameObject* AddModelObject(const std::filesystem::path& modelPath);

		void ClearAll();

		bool SaveToFile(const std::filesystem::path& filepath);
		bool LoadFromFile(const std::filesystem::path& filepath);
	
		const std::vector<std::unique_ptr<GameObject>>& GetRoots() const { return m_roots; }
		void AddRoot(std::unique_ptr<GameObject> go);
		void RemoveRoot(GameObject* go);
		bool IsRoot(GameObject* go);
		
		const std::vector<GameObject*>& GetAll()const { return m_all; }

		bool IsDirty() const { return m_isDirty; }
		void SetDirty() { m_isDirty = true; }
		void ClearDirty() { m_isDirty = false; }

	private:
		std::vector<std::unique_ptr<GameObject>> m_roots;
		std::vector<GameObject*> m_all;

		bool m_isDirty = false;
	};
}