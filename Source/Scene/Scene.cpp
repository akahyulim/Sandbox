#include "pch.h"
#include "Scene.h"
#include "Resource/ResourceManager.h"
#include "Resource/StaticMesh.h"
#include "Resource/Material.h"
#include "Components/MeshRenderer.h"
#include "Components/Camera.h"
#include "Components/Transform.h"
#include "Components/Light.h"

namespace Dive
{
	Scene::Scene() = default;
	Scene::~Scene() = default;

	void Scene::Update(float dt)
	{
		std::function<void(GameObject*)> updateNode = [&](GameObject* go) {
			if (!go->IsActive())
				return;

			go->Update(dt);

			for (auto& child : go->GetChildren())
				updateNode(child.get());
			};

		for (const auto& go : m_roots)
			updateNode(go.get());
	}

	GameObject* Scene::CreateGameObject(uint64_t id)
	{
		auto newGO = std::make_unique<GameObject>(id, this);
		GameObject* ptr = newGO.get();
		m_all.push_back(ptr);
		m_roots.push_back(std::move(newGO));

		SetDirty();

		return ptr;
	}

	GameObject* Scene::AddPresetObject(ePresetType type)
	{
		auto mesh = ResourceManager::GetInst().GetPresetMesh(type);
		auto mat = ResourceManager::GetInst().Get<Material>("Default_Material");

		auto ptr = CreateGameObject(); 
		auto com = ptr->AddComponent<MeshRenderer>();
		com->SetMaterial(mat);
		com->SetMesh(mesh);

		return ptr;
	}
	
	void Scene::ClearAll()
	{
		//m_drawables.clear();
		m_all.clear();
		m_roots.clear();

		SetDirty();
	}

	bool Scene::SaveToFile(const std::filesystem::path& filepath)
	{
		return false;
	}
	
	bool Scene::LoadFromFile(const std::filesystem::path& filepath)
	{
		//m_objects.clear();


		return false;
	}

	void Scene::AddRoot(std::unique_ptr<GameObject> go)
	{
		if (go == nullptr)
			return;

		if (IsRoot(go.get()))
			return;

		m_roots.emplace_back(std::move(go));
		
		SetDirty();
	}

	void Scene::RemoveRoot(GameObject* go)
	{
		if (go == nullptr || m_roots.empty())
			return;

		auto it = std::find_if(m_roots.begin(), m_roots.end(),
			[go](const auto& ptr) { return ptr.get() == go; });

		if (it != m_roots.end())
		{
			go->m_parent = nullptr;
			m_roots.erase(it);
		}

		SetDirty();
	}

	bool Scene::IsRoot(GameObject* go)
	{
		if (go == nullptr)
			return false;

		auto it = std::find_if(m_roots.begin(), m_roots.end(),
			[go](const auto& ptr) { return ptr.get() == go; });

		return it != m_roots.end();
	}
}