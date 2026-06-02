#pragma once
#include <string>
#include <cstdint>
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "Core/Object.h"
#include "GameObject.h"

namespace Dive
{
	struct Ray;
	struct RaycastHit;

	class Camera;
	class Light;
	class Transform;
	class MeshRenderer;
	class Model;

	struct LightBuckets
	{
		Light* directionalLight = nullptr;
		std::vector<Light*> pointLights;
		std::vector<Light*> spotLights;

		void Clear()
		{
			directionalLight = nullptr;
			pointLights.clear();
			spotLights.clear();
		}
	};
	
	struct RenderQueue
	{
		std::vector<MeshRenderer*> opaques;
		std::vector<MeshRenderer*> transparents;

		void Clear()
		{
			opaques.clear();
			transparents.clear();
		}
	};

	struct RenderPacket
	{
		LightBuckets lights;
		RenderQueue meshes;
	};

	class Scene
	{
	public:
		Scene() = default;
		Scene(const std::string& name);
		~Scene();

		void Clear();

		void Update();

		void CullAndSort(Camera* camera);
		RenderPacket NewCullAndSort(Camera* camera);

		GameObject* Instantiate(std::shared_ptr<Model> model, uint64_t rootId = AUTO_ID);

		GameObject* CreateGameObject(const std::string& name = "GameObject", uint64_t id = AUTO_ID);
		GameObject* CreatePresetGameObject(ePresetType type);
		GameObject* RegisterGameObject(std::unique_ptr<GameObject> obj);

		void DestroyGameObject(GameObject* obj, bool destroyChildren);
		void QueueDestroy(GameObject* obj);
		void FlushDestoryQueue();

		bool HasGameObject(GameObject* obj);
		bool HasGameObject(uint64_t id);

		GameObject* FindGameObject(uint64_t id);

		bool Raycast(const Ray& ray, RaycastHit* outHit, bool allowInsideHits = false) const;

		size_t AllGameObjectCount() const { return m_gameObjectMap.size(); }
		std::vector<GameObject*> GetAllGameObjects();

		size_t RootGameObjectCount() const { return m_rootGameObjects.size(); }
		const std::vector<GameObject*>& GetRootGameObjects() { return m_rootGameObjects; }
		
		const std::vector<Light*>& GetLights() const { return m_lights; }

		const std::vector<MeshRenderer*>& GetTransparentMeshRenderers() const { return m_transparents; }
		const std::vector<MeshRenderer*>& GetOpaqueMeshRenderers() const { return m_opaques; }

		std::string GetName() const { return m_name; }
		void SetName(const std::string& name) { m_name = name; }

		void NotifyParentChanged(GameObject* obj);

	private:
		std::string m_name{};

		std::unordered_map<uint64_t, std::unique_ptr<GameObject>> m_gameObjectMap;
		std::vector<GameObject*> m_rootGameObjects;
		std::unordered_set<uint64_t> m_destroyQueue;

		std::vector<Light*> m_lights;

		std::vector<MeshRenderer*> m_transparents;
		std::vector<MeshRenderer*> m_opaques;

		// 메인 카메라 정도는 관리했으면 좋겠다.

		friend class Transform;
	};
}
