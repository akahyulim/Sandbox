#include "pch.h"
#include "Scene.h"
#include "GameObject.h"
#include "Components/MeshRenderer.h"
#include "Components/Camera.h"
#include "Components/Transform.h"
#include "Components/Light.h"

namespace Dive
{
	Scene::Scene()
	{
		// 일단 그냥 생성
		m_camera = new GameObject(this);
		m_camera->SetName("MainCamera");
		m_camera->AddComponent<Camera>();
		m_camera->GetTransform()->SetPosition(0.0f, .0f, -5.0f);
		//m_camera->GetTransform()->SetRotationByDegrees({ 0.0f, 180.0f, 0.0f });
	}

	Scene::~Scene()
	{
		delete m_camera;
	}

	void Scene::Clear()
	{
		m_gameObjectMap.clear();

		SetDirty();
	}
	
	// spartan에서는 기본적으로 추가된 Entity를 pending에 보관하고
	// dirty를 확인한 후 좀 더 세부적(render, lights, cameras, ragdoll, particles 등)으로 분류한다.
	// 일단 renderable을 opaque와 tansparent로 구분해보자.
	// 그러려면 GameObject - MeshRenderer 그리고 Mesh, Material 등을 다시 조정해야 한다.
	void Scene::Update()
	{
		// temp
		if (m_camera)
			m_camera->Update();

		if (!m_pendingAdditions.empty())
		{
			for (auto& gameObject : m_pendingAdditions)
			{
				uint64_t id = gameObject->GetInstanceID();
				m_gameObjectMap.insert({ id, std::move(gameObject) });
			}

			m_pendingAdditions.clear();

			SetDirty();
		}

		if (!m_pendingRemovals.empty())
		{
			for (auto& gameObject : m_pendingRemovals)
			{
				// 부모와의 관계는 끊되, 소멸자에서 다시 RemoveChild를 부르지 않도록
				// GameObject에 'Scene에서 삭제 중'이라는 상태를 잠시 알려주는 것이 좋습니다.
				//gameObject->SetPendingDeletion(true);
				
				if (gameObject->HasParent())
					gameObject->DetachFromParent();

				m_gameObjectMap.erase(gameObject->GetInstanceID());
			}

			m_pendingRemovals.clear();

			SetDirty();
		}

		for (auto& [id, gameObect] : m_gameObjectMap)
		{
			if (gameObect->IsActive())
			{
				gameObect->Update();
			}
		}

		if (m_isDirty)
		{
			m_lights.clear();
			m_renderables.clear();

			for (auto& [id, gameObect] : m_gameObjectMap)
			{
				if (gameObect->IsActive())
				{
					//if (m_camera == nullptr && gameObect->GetComponent<Camera>())
					{
					//	m_camera = gameObect.get();
					}

					if (auto light = gameObect->GetComponent<Light>())
					{
						if (m_directionalLight == nullptr && light->GetLightType() == eLightType::Directional)
						{
							m_directionalLight = gameObect.get();
						}
						else
						{
							m_lights.emplace_back(gameObect.get());
						}
					}

					if (gameObect->GetComponent<MeshRenderer>())
					{
						m_renderables.emplace_back(gameObect.get());
					}
				}
			}

			ClearDirty();
		}
	}

	GameObject* Scene::CreateGameObject()
	{
		auto gameObject = std::make_unique<GameObject>(this);
		gameObject->AddComponent<Transform>();

		auto rawPtr = gameObject.get();

		m_pendingAdditions.emplace_back(std::move(gameObject));

		return rawPtr;
	}

	GameObject* Scene::CreateGameObjectFromID(uint64_t id)
	{
		auto gameObject = std::make_unique<GameObject>(this, id);
		gameObject->AddComponent<Transform>();

		auto rawPtr = gameObject.get();

		m_pendingAdditions.emplace_back(std::move(gameObject));

		return rawPtr;
	}

	void Scene::RemoveGameObject(GameObject* gameObject)
	{
		assert(gameObject);

		auto it = std::find(m_pendingRemovals.begin(), m_pendingRemovals.end(), gameObject);
		if (it != m_pendingRemovals.end())
			return;

		m_pendingRemovals.emplace_back(gameObject);
		gameObject->GetDecendants(m_pendingRemovals);
	}

	void Scene::RemoveGameObjectByID(uint64_t id)
	{
		auto it = m_gameObjectMap.find(id);
		if (it != m_gameObjectMap.end())
			RemoveGameObject(it->second.get());
	}

	void Scene::GetRootGameObjects(std::vector<GameObject*>& outRoots)
	{
		outRoots.clear();
		outRoots.reserve(m_gameObjectMap.size());

		for (auto& [id, gameObect] : m_gameObjectMap)
		{
			if (!gameObect->HasParent())
				outRoots.emplace_back(gameObect.get());
		}
	}

	bool Scene::SaveToFile(const std::filesystem::path& filepath)
	{
		return false;
	}

	bool Scene::LoadFromFile(const std::filesystem::path& filepath)
	{

		return false;
	}
}