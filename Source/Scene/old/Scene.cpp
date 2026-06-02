#include "pch.h"
#include "Scene.h"
//#include "SceneSerializer.h"
//#include "Collision.h"
//#include "Components/Transform.h"
//#include "Components/Camera.h"
//#include "Components/Light.h"
//#include "Components/MeshRenderer.h"
//#include "Rendering/RenderingSystem.h"
//#include "Resource/ResourceSystem.h"
#include "Core/EventDispatcher.h"

using namespace DirectX;

namespace Dive
{
	static float ComputeSortKey(const MeshRenderer* meshRenderer, const Camera* camera)
	{
		const auto& boundingBox = meshRenderer->GetBounds();
		XMFLOAT3 center = boundingBox.center;

		XMVECTOR camPos = camera->GetTransform()->GetPositionVector();
		XMVECTOR objPos = XMLoadFloat3(&center);
		XMVECTOR diff = XMVectorSubtract(objPos, camPos);
		XMVECTOR distSq = XMVector3LengthSq(diff);

		float distance;
		XMStoreFloat(&distance, distSq);
		return distance;
	}

	Scene::Scene(const std::string& name)
		: m_name(name)
	{
	}

	Scene::~Scene()
	{
		Clear();
	}

	void Scene::Clear()
	{
		m_destroyQueue.clear();
		m_rootGameObjects.clear();
		m_gameObjectMap.clear();
	}
	
	// 기존보다 더 버벅이는 듯하다.
	void Scene::Update()
	{
		for (auto& root : m_rootGameObjects)
		{
			root->Update();
		}

		DV_FIRE_EVENT(eEventType::Update);
		DV_FIRE_EVENT(eEventType::LateUpdate);
	}

	// 정렬 및 절두체 컬링을 위해 카메라를 사용하므로
	// Update가 아니 개별 호출 메서드로 만들었다.
	// 허나 현재 위의 기능을 하지 못한다.
	void Scene::CullAndSort(Camera* camera)
	{
		assert(camera);

		m_lights.clear();
		m_opaques.clear();
		m_transparents.clear();

		const auto& frustum = camera->GetFrustum();
		for (auto& [id, obj] : m_gameObjectMap)
		{
			if (!obj->IsActiveSelf() || !obj->IsActiveHierarchy())
				continue;

			if (obj->HasComponent<Light>())
			{
				m_lights.push_back(obj->GetComponent<Light>());
			}
			else if (obj->HasComponent<MeshRenderer>())
			{
				auto meshRenderer = obj->GetComponent<MeshRenderer>();
				auto material = meshRenderer->GetMaterial();

				if (meshRenderer->IsVisible(frustum))
				{
					switch (material->GetRenderingMode())
					{
					case eRenderingMode::Opqaue:
						m_opaques.push_back(meshRenderer);
						break;
					case eRenderingMode::Transparent:
						m_transparents.push_back(meshRenderer);
						break;
					default:
						break;
					}
				}
			}
		}

		std::sort(m_opaques.begin(), m_opaques.end(),
			[camera](MeshRenderer* a, MeshRenderer* b)
			{
				return ComputeSortKey(a, camera) < ComputeSortKey(b, camera); // 거리 오름차순 (선택적)
			});

		std::sort(m_transparents.begin(), m_transparents.end(),
			[camera](MeshRenderer* a, MeshRenderer* b)
			{
				return ComputeSortKey(a, camera) > ComputeSortKey(b, camera); // 거리 내림차순
			});
	}

	RenderPacket Scene::NewCullAndSort(Camera* camera)
	{
		assert(camera);

		RenderPacket packet;

		const auto& frustum = camera->GetFrustum();
		for (auto& [id, obj] : m_gameObjectMap)
		{
			if (!obj->IsActiveSelf() || !obj->IsActiveHierarchy())
				continue;

			if (obj->HasComponent<Light>())
			{
				auto light = obj->GetComponent<Light>();
				switch (light->GetLightType())
				{
				case eLightType::Directional:
					if(packet.lights.directionalLight == nullptr)
						packet.lights.directionalLight = light;
					break;
				case eLightType::Point:
					if (light->IsVisible(frustum))
						packet.lights.pointLights.push_back(light);
					break;
				case eLightType::Spot:
					if (light->IsVisible(frustum))
						packet.lights.spotLights.push_back(light);
					break;
				}
			}

			if (obj->HasComponent<MeshRenderer>())
			{
				auto meshRenderer = obj->GetComponent<MeshRenderer>();
				auto material = meshRenderer->GetMaterial();

				if (meshRenderer->IsVisible(frustum))
				{
					switch (material->GetRenderingMode())
					{
					case eRenderingMode::Opqaue:
						packet.meshes.opaques.push_back(meshRenderer);
						break;
					case eRenderingMode::Transparent:
						packet.meshes.transparents.push_back(meshRenderer);
						break;
					default:
						break;
					}
				}
			}
		}

		std::sort(packet.meshes.opaques.begin(), packet.meshes.opaques.end(),
			[camera](MeshRenderer* a, MeshRenderer* b)
			{
				return ComputeSortKey(a, camera) < ComputeSortKey(b, camera); // 거리 오름차순 (선택적)
			});

		std::sort(packet.meshes.transparents.begin(), packet.meshes.transparents.end(),
			[camera](MeshRenderer* a, MeshRenderer* b)
			{
				return ComputeSortKey(a, camera) > ComputeSortKey(b, camera); // 거리 내림차순
			});

		return packet;
	}

	GameObject* Scene::Instantiate(std::shared_ptr<Model> model, uint64_t rootId)
	{
		if (!model) 
			return nullptr;

		const auto& modelNodes = model->GetNodes();
		std::vector<GameObject*> gameObjects;
		gameObjects.reserve(modelNodes.size());

		GameObject* rootObject = nullptr;

		for (size_t i = 0; i < modelNodes.size(); ++i)
		{
			const auto& modelNode = modelNodes[i];

			bool isRoot = (modelNode.parentIndex == -1);
			uint64_t targetId = isRoot ? rootId : GenerateUniqueID();

			auto instance = std::make_unique<GameObject>(this, modelNode.name, targetId);
			GameObject* object = instance.get();

			RegisterGameObject(std::move(instance));
			gameObjects.push_back(object);

			Transform* transform = object->GetTransform();
			transform->SetLocalTransform(modelNode.localTransform);

			auto info = std::make_unique<ModelInstanceInfo>();
			info->modelNodeName = modelNode.name;
			info->isModified = false;

			if (isRoot) 
			{
				rootObject = object;
				info->isRoot = true;
				info->modelRoot = object;
				info->modelPath = model->GetFilepath().string();
			}
			else 
			{
				GameObject* parentObject = gameObjects[modelNode.parentIndex];
				transform->SetParent(parentObject->GetTransform());

				info->isRoot = false;
				info->modelRoot = rootObject;
			}

			object->SetModelInfo(std::move(info));

			if (!modelNode.meshInstances.empty())
			{
				MeshRenderer* meshRenderer = object->AddComponent<MeshRenderer>();
				meshRenderer->SetStaticMesh(std::static_pointer_cast<StaticMesh>(modelNode.meshInstances[0].mesh));
				meshRenderer->SetMaterial(modelNode.meshInstances[0].material);
			}
		}

		return rootObject;
	}

	GameObject* Scene::CreateGameObject(const std::string& name, uint64_t id)
	{
		auto obj = std::make_unique<GameObject>(this, name, id);
		auto rawPtr = obj.get();

		m_gameObjectMap[obj->GetInstanceID()] = std::move(obj);
		m_rootGameObjects.push_back(rawPtr);

		return rawPtr;
	}

	GameObject* Scene::CreatePresetGameObject(ePresetType type)
	{
		auto name = Preset::ToString(type);

		auto instance = std::make_unique<GameObject>(this, name);
		GameObject* rawPtr = instance.get();

		auto meshRenderer = instance->AddComponent<MeshRenderer>();

		auto presetMeshKey = "Preset_" + name;
		auto mesh = Resources::Load<StaticMesh>(presetMeshKey);
		auto mat = Resources::Load<Material>("Materials/Default");
		
		meshRenderer->SetStaticMesh(mesh);
		meshRenderer->SetMaterial(mat);
		meshRenderer->SetPresetType(type);

		uint64_t id = instance->GetInstanceID();
		m_gameObjectMap[id] = std::move(instance);
		m_rootGameObjects.push_back(rawPtr);

		return rawPtr;
	}

	GameObject* Scene::RegisterGameObject(std::unique_ptr<GameObject> obj)
	{
		if (!obj)	
			return nullptr;

		auto rawPtr = obj.get();
		uint64_t id = obj->GetInstanceID();

		if (HasGameObject(rawPtr))
		{
			DV_LOG(Scene, eLogLevel::Err, "이미 등록된 게임오브젝트: {} - {}", rawPtr->GetName(), id);
			return nullptr;
		}

		rawPtr->SetWorld(this);
		m_gameObjectMap[id] = std::move(obj);
		
		if (!rawPtr->GetParent())
		{
			m_rootGameObjects.push_back(rawPtr);
		}

		return rawPtr;
	}

	std::vector<GameObject*> Scene::GetAllGameObjects()
	{
		std::vector<GameObject*> allGameObjects;
		allGameObjects.reserve(m_gameObjectMap.size());

		for (auto& [id, obj] : m_gameObjectMap)
			allGameObjects.push_back(obj.get());

		return allGameObjects;
	}

	void Scene::NotifyParentChanged(GameObject* obj)
	{
		if (!obj)
			return;

		auto transform = obj->GetTransform();
		auto it = std::find(m_rootGameObjects.begin(), m_rootGameObjects.end(), obj);

		if (transform->GetParent() == nullptr)
		{
			if(it == m_rootGameObjects.end())
				m_rootGameObjects.push_back(obj);
		}
		else
		{
			if (it != m_rootGameObjects.end())
				m_rootGameObjects.erase(it);
		}
	}

	void Scene::DestroyGameObject(GameObject* obj, bool destroyChildren)
	{
		if (destroyChildren)
		{
			for (auto child : obj->GetTransform()->GetChildren())
				DestroyGameObject(child->GetGameObject(), destroyChildren);
		}

		QueueDestroy(obj);
	}

	void Scene::QueueDestroy(GameObject* obj)
	{
		assert(obj);
		assert(HasGameObject(obj->GetInstanceID()));

		if (m_destroyQueue.find(obj->GetInstanceID()) == m_destroyQueue.end())
		{
			m_destroyQueue.insert(obj->GetInstanceID());
			obj->GetTransform()->UnsubscribeEvents();	// 별도로 해줘야 한다.

			for (auto& [type, component] : obj->m_components)
			{
				component->UnsubscribeEvents();
			}
		}
	}

	void Scene::FlushDestoryQueue()
	{
		for (auto id : m_destroyQueue)
		{
			auto it = m_gameObjectMap.find(id);
			if (it != m_gameObjectMap.end())
			{
				auto root = std::find(m_rootGameObjects.begin(), m_rootGameObjects.end(), it->second.get());
				if (root != m_rootGameObjects.end())
					m_rootGameObjects.erase(root);

				m_gameObjectMap.erase(it);
			}
		}
	}

	bool Scene::HasGameObject(GameObject* obj)
	{
		if(!obj)
			return false;

		return HasGameObject(obj->GetInstanceID());
	}

	bool Scene::HasGameObject(uint64_t id)
	{
		return m_gameObjectMap.find(id) != m_gameObjectMap.end();
	}

	GameObject* Scene::FindGameObject(uint64_t id)
	{
		auto it = m_gameObjectMap.find(id);
		return it != m_gameObjectMap.end() ? it->second.get() : nullptr;
	}

	/*
	// 기본적으로 Box 내부에서는 대상을 제외
	bool Scene::Raycast(const Ray& ray, RaycastHit* outHit, bool allowInsideHits) const
	{
		assert(outHit);
		outHit->distance = FLT_MAX;
		outHit->hitObject = nullptr;

		auto tryHit = [&](const std::vector<MeshRenderer*>& renderers)
			{
				for (auto& pRenderer : renderers)
				{
					const auto& boundingBox = pRenderer->GetBounds();

					if (!allowInsideHits && Collision::IsPointInsideAABB(ray.origin, boundingBox.center, boundingBox.extents))
						continue;

					float hitDistance;
					XMFLOAT3 hitPoint;
					XMFLOAT3 hitNormal;

					if (pRenderer->Intersects(ray, &hitDistance, &hitPoint, &hitNormal))
					{
						if (hitDistance < outHit->distance)
						{
							outHit->distance = hitDistance;
							outHit->point = hitPoint;
							outHit->normal = hitNormal;
							outHit->hitObject = pRenderer->GetGameObject();
						}
					}
				}
			};

		tryHit(m_opaques);
		tryHit(m_transparents);

		return outHit->hitObject != nullptr;
	}
	*/
	bool Scene::Raycast(const Ray& ray, RaycastHit* outHit, bool allowInsideHits) const
	{
		assert(outHit);
		outHit->distance = FLT_MAX;
		outHit->hitObject = nullptr;

		// 계층 구조 순회를 위한 람다 함수
		std::function<void(GameObject*)> findClosestHit = [&](GameObject* gameObject)
			{
				if (!gameObject->IsActiveSelf()) return;

				if (auto* renderer = gameObject->GetComponent<MeshRenderer>())
				{
					const auto& bounds = renderer->GetBounds();
					float tMin;
					XMFLOAT3 tempNormal;

					// 1. 작성하신 IntersectRayAABB를 사용하여 가벼운 박스 검사 수행
					if (Collision::IntersectRayAABB(ray, bounds.center, bounds.extents, &tMin, &tempNormal))
					{
						// [최적화] AABB 충돌 거리가 이미 찾은 최단 거리보다 멀다면 정밀 검사 필요 없음
						if (tMin < outHit->distance)
						{
							// [조건] 내부 충돌 허용 여부 (tMin이 0에 가깝다면 광원이 박스 안에 있는 것)
							if (!allowInsideHits && tMin <= 0.0001f)
							{
								// 내부 진입 시 스킵
							}
							else
							{
								// 2. 실제 삼각형 단위 정밀 검사 (AABB는 맞았으므로 실제 메쉬와 충돌 확인)
								float meshDist;
								XMFLOAT3 meshPoint, meshNormal;
								if (renderer->Intersects(ray, &meshDist, &meshPoint, &meshNormal))
								{
									// 3. 최종 최단 거리 갱신
									if (meshDist < outHit->distance)
									{
										outHit->distance = meshDist;
										outHit->point = meshPoint;
										outHit->normal = meshNormal;
										outHit->hitObject = gameObject;
									}
								}
							}
						}
					}
				}

				// 자식들 탐색
				for (auto* child : gameObject->GetTransform()->GetChildren())
				{
					findClosestHit(child->GetGameObject());
				}
			};

		for (auto* rootObj : m_rootGameObjects)
		{
			findClosestHit(rootObj);
		}

		return outHit->hitObject != nullptr;
	}
}
