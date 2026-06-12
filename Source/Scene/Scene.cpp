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
	Scene::Scene()
	{
		m_mainCamera = std::make_unique<GameObject>();
		m_mainCamera->AddComponent<Camera>();

		m_dirLight = std::make_unique<GameObject>();
		Light* lightCom = m_dirLight->AddComponent<Light>();
		lightCom->SetLightType(eLightType::Directional);
		lightCom->SetColor(Color::White);
		//lightCom->SetDirection(-1.0f, -1.0f, 1.0f);
		lightCom->SetDirection(0.5f, -0.5f, 0.707107f);
	}

	Scene::~Scene()
	{
	}

	void Scene::Update(float dt)
	{
		m_mainCamera->Update();

		for (const auto& obj : m_objects)
		{
			if (obj->IsActive() && obj->GetTransform()->GetParent() == nullptr)
			{
				// 이 부분을 재귀함수로 만들어 놓았다.
				{
					obj->Update();

					const auto& children = obj->GetTransform()->GetChildren();
					for (auto* child : children)
					{
						GameObject* childObj = child->GetOwner();
						if (childObj && childObj->IsActive())
						{
							// 재귀호출
						}
					}
				}
			}
		}
	}

	void Scene::PrepareRenderChannels()
	{
		m_pointLights.clear();
		m_spotLights.clear();
		m_opaqueRenderers.clear();
		m_transparentRenderers.clear();

		// extract render channels
		{
			// 메인 카메라 획득
			// 메인 카메라로부터 프러스텀 획득

			for (const auto& obj : m_objects)
			{
				if (!obj->IsActive())
					continue;

				if (auto renderer = obj->GetComponent<MeshRenderer>())
				{
					// 바운딩 박스 획득
					// 컬링 확인

					// 머티리얼 획득
					// opaque, transparent 분류 저장
				}
			}
		}

		// sort renderers
		{

		}
	}

	GameObject* Scene::AddLightObject(eLightType type)
	{
		if (type == eLightType::Directional)
			return nullptr;

		auto gameObject = std::make_unique<GameObject>();

		Light* light = gameObject->AddComponent<Light>();
		light->SetLightType(type);

		GameObject* ptr = gameObject.get();
		m_objects.push_back(std::move(gameObject));

		return ptr;
	}

	GameObject* Scene::AddPresetObject(ePresetType type)
	{
		// 게임 오브젝트 생성
		auto gameObject = std::make_unique<GameObject>();

		// 메시 가져오기 - 리소스 매니져
		//=> 이것도 ResourceManager::Initialize에서 초기화하고 Get으로 획득하기
		auto mesh = ResourceManager::GetInst().GetPresetMesh(type);
		// 머티리얼 가져오기 - 리소스 매니져
		auto mat = ResourceManager::GetInst().Get<Material>("Default_Material");

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