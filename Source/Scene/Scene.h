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
		void PrepareRenderChannels();

		GameObject* AddLightObject(eLightType type);
		GameObject* AddPresetObject(ePresetType type);
		GameObject* AddModelObject(const std::filesystem::path& modelPath);

		void DeleteSelectedObject();
		void ClearAll();

		bool SaveToFile(const std::filesystem::path& filepath);
		bool LoadFromFile(const std::filesystem::path& filepath);

		Color GetClearColor() const { return m_clearColor; }
		void SetClearColor(const Color& color) { m_clearColor = color; }

		GameObject* GetMainCamera() const { return m_mainCamera.get(); }
		GameObject* GetDirectionalLight() const { return m_dirLight.get(); }
		const std::vector<std::unique_ptr<GameObject>>& GetGameObjects() const { return m_objects; }

		GameObject* GetSelectedObject() const { return m_selectedObject; }
		void SetSelectedObject(GameObject* target) { m_selectedObject = target; }

		std::vector<GameObject*> GetDrawable();


	private:
		Color m_clearColor = Color::White;

		std::unique_ptr<GameObject> m_mainCamera;
		std::unique_ptr<GameObject> m_dirLight;
		std::vector<std::unique_ptr<GameObject>> m_objects;
		
		GameObject* m_selectedObject = nullptr;

		std::vector<Light*> m_pointLights;
		std::vector<Light*> m_spotLights;
		std::vector<MeshRenderer*> m_opaqueRenderers;
		std::vector<MeshRenderer*> m_transparentRenderers;
	};
}