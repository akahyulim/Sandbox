#include "pch.h"
#include "Scene.h"
#include "Resource/ResourceManager.h"
#include "Resource/StaticMesh.h"
#include "Resource/Material.h"
#include "Components/MeshRenderer.h"
#include "Components/Camera.h"

namespace Dive
{
	Scene::Scene()
	{
		m_mainCamera = std::make_unique<GameObject>();
		m_mainCamera->AddComponent<Camera>();
	}

	Scene::~Scene()
	{
	}

	void Scene::Update(float dt)
	{
		m_mainCamera->Update();

		for (auto& obj : m_objects)
		{
			obj->Update();
		}
	}

	GameObject* Scene::AddPresetObject(ePresetType type)
	{
		// 게임 오브젝트 생성
		auto gameObject = std::make_unique<GameObject>();

		// 메시 가져오기 - 리소스 매니져
		auto mesh = ResourceManager::GetInst().GetPresetMesh(type);
		// 머티리얼 가져오기 - 리소스 매니져
		auto mat = std::make_shared<Material>();

		// 메시 렌더러 생성 후 메시, 머티리얼 연결
		auto com = gameObject->AddComponent<MeshRenderer>();
		com->SetMaterial(mat);
		com->SetMesh(mesh);

		// 게임 오브젝트 등록 후 포인터 리턴
		GameObject* ptr = gameObject.get();
		m_objects.push_back(std::move(gameObject));

		return ptr;
	}
	
	void Scene::DeleteSelectedObject()
	{
		if (m_selectedObject == nullptr)
			return;

		m_objects.erase(
			std::remove_if(m_objects.begin(), m_objects.end(),
				[this](const std::unique_ptr<GameObject>& obj)
				{
					return obj.get() == m_selectedObject;
				}),
			m_objects.end()
		);

		m_selectedObject = nullptr;
	}
	
	void Scene::ClearAll()
	{
		m_selectedObject = nullptr;
		m_objects.clear();
		m_mainCamera.reset();
	}

	bool Scene::SaveToFile(const std::filesystem::path& filepath)
	{
		return false;
	}
	
	bool Scene::LoadFromFile(const std::filesystem::path& filepath)
	{
		m_objects.clear();


		return false;
	}

	std::vector<GameObject*> Scene::GetDrawable()
	{
		std::vector<GameObject*> drawable;

		for (auto& gameObject : m_objects)
		{
			drawable.push_back(gameObject.get());
		}

		return drawable;
	}
}