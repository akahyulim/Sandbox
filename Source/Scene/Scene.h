#pragma once
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <filesystem>

#include "Core/Types.h"
#include "Core/Object.h"
#include "Utilities/Math.h"
#include "Graphics/ConstantBufferDatas.h"
#include "Rendering/TextureManager.h"

namespace Dive
{
	class GameObject;

	class Camera;
	class Light;
	class MeshRenderer;

	enum class eSkyMode : uint8_t
	{
		Skybox,
		SkySphere,
		UniformColor
	};

	struct Environment
	{
		bool operator==(const Environment& other) const
		{
			return (skyMode == other.skyMode) &&
				(skyboxCubemap == other.skyboxCubemap) &&
				(skyboxPath == other.skyboxPath) &&
				Math::XMFLOAT4Equal(data.apexColor, other.data.apexColor) &&
				Math::XMFLOAT4Equal(data.horizonColor, other.data.horizonColor) &&
				Math::XMFLOAT4Equal(data.lightDir, other.data.lightDir) &&
				Math::XMFLOAT4Equal(data.lightColor, other.data.lightColor) &&
				Math::XMFLOAT4Equal(data.ambientColor, other.data.ambientColor);
		}

		bool operator!=(const Environment& other) const
		{
			return !(*this == other);
		}

		eSkyMode skyMode = eSkyMode::Skybox;
		TextureHandle skyboxCubemap = INVALID_TEXTURE_HANDLE;
		std::string skyboxPath = "";
		EnvironmentData data;
	};

	class Scene
	{
	public:
		Scene();
		~Scene();

		void Clear();

		void Update();
		
		GameObject* CreateGameObject();
		GameObject* CreateGameObject(const std::string& name, uint64_t id = AUTO_ID);
		GameObject* CreateGameObjectFromID(uint64_t id);
		void RemoveGameObject(GameObject* gameObject);
		void RemoveGameObjectByID(uint64_t id);

		void GetRootGameObjects(std::vector<GameObject*>& outRoots);
		std::vector<GameObject*> GetRootGameObjects();	// 복사 리턴 추가

		GameObject* GetGameObjectByObjectID(uint32_t objectID);

		GameObject* GetCamera() const { return m_camera; }
		const std::vector<GameObject*>& GetLightQueue() const { return m_lights; }
		const std::vector<GameObject*>& GetRenderables() const { return m_renderables; }
		std::vector<GameObject*> GetAllGameObjects();

		bool IsDirty() const { return m_isDirty; }
		void SetDirty() { m_isDirty = true; }
		void ClearDirty() { m_isDirty = false; }

		std::string GetName() const { return m_name; }
		void SetName(const std::string& name) { m_name = name; }

		Environment GetEnvironment() const { return m_environment; }
		void SetEnvironment(const Environment& data);

	private:
		std::string m_name = "Playgorund";
			
		std::vector<std::unique_ptr<GameObject>> m_pendingAdditions;
		std::unordered_map<uint64_t, std::unique_ptr<GameObject>> m_gameObjectMap;
		std::vector<GameObject*> m_pendingRemovals;
		
		GameObject* m_camera = nullptr;
		std::vector<GameObject*> m_lights;
		std::vector<GameObject*> m_renderables;
		// particles

		bool m_isDirty = false;

		Environment m_environment;
	};
}