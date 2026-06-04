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

	class Scene
	{
	public:
		Scene();
		~Scene();

		void Update(float dt);

		GameObject* AddPresetObject(ePresetType type);
		GameObject* AddModelObject(const std::filesystem::path& modelPath);

		void DeleteSelectedObject();
		void ClearAll();

		bool SaveToFile(const std::filesystem::path& filepath);
		bool LoadFromFile(const std::filesystem::path& filepath);

		Color GetClearColor() const { return m_clearColor; }
		void SetClearColor(const Color& color) { m_clearColor = color; }

		GameObject* GetMainCamera() { return m_mainCamera.get(); }
		const std::vector<std::unique_ptr<GameObject>>& GetGameObjects() const { return m_objects; }

		GameObject* GetSelectedObject() const { return m_selectedObject; }
		void SetSelectedObject(GameObject* target) { m_selectedObject = target; }

		std::vector<GameObject*> GetDrawable();

	private:

	private:
		Color m_clearColor = Color::White;

		std::unique_ptr<GameObject> m_mainCamera;
		std::vector<std::unique_ptr<GameObject>> m_objects;
		GameObject* m_selectedObject = nullptr;
		
	};
}