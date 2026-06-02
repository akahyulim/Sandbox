#include "pch.h"
#include "GameObject.h"
#include "Scene.h"
#include "Components/Light.h"
#include "Components/Camera.h"
#include "Components/MeshRenderer.h"
//#include "Resource/ResourceSystem.h"
//#include "Rendering/RenderingSystem.h"

namespace Dive
{
	GameObject::GameObject(const std::string& name, uint64_t id)
		: Object(id)
	{
		SetName(name);
		m_transform = std::make_unique<Transform>(this);
	}

	GameObject::GameObject(Scene* scene, const std::string& name, uint64_t id)
		: Object(id)
		, m_scene(scene)
	{
		SetName(name);
		m_transform = std::make_unique<Transform>(this);
	}

	GameObject::~GameObject()
	{
		m_components.clear();
		m_transform.release();
	}

	void GameObject::Update()
	{
		// 버그: 역직렬화 직후 자식들은 isActiveHierarchy가 갱신되지 않아
		// 계층구조가 초기화된 상태에서 보여지기도 한다.
		if (!m_isActiveHierarchy || m_isDestroyed)
			return;

		m_transform->Update();

		for (auto& [type, component] : m_components)
		{
			component->Update();
		}

		// 이걸 왜 별도로 분리했을까? 기억나지 않는다...
		// 아마도 MonoBehaviour는 GameObject 간의 상호관계때문이었던 것 같기도...
		//for (auto& script : m_scripts)
		{
		//	script->Update();
		}

		auto& children = m_transform->GetChildren();
		for (auto child : children)
		{
			if (auto owner = child->GetGameObject())
				owner->Update();
		}
	}

	std::unique_ptr<GameObject> GameObject::Clone(Scene* scene, uint64_t id) const
	{
		// 사실 여기에서 Scene의 CreateGameObject를 호출해도 된다.
		// 상호참조때문에 이렇게 했던 것 같다.
		// 이때문에 Prefab의 Clone에서 리턴받은 go를 다시 AddGameObject로 등록한다.
		auto clone = std::make_unique<GameObject>(scene, GetName(), id);
		
		//clone->SetName(GetName());
		//clone->m_presetType = m_presetType;
		//clone->m_prefab = m_prefab;
		clone->m_tag = m_tag;
		//clone->m_isPrefabRoot = m_isPrefabRoot;
		clone->m_isDestroyed = m_isDestroyed;
		clone->m_isActiveSelf = m_isActiveSelf;
		clone->m_isActiveHierarchy = m_isActiveHierarchy;

		for (auto& [type, comp] : m_components)
		{
			clone->m_components[type] = comp->Clone(clone.get());
		}

		return clone;
	}

	void GameObject::Serialize(YAML::Emitter& out)
	{
		out << YAML::Key << "ID" << YAML::Value << GetInstanceID();
		out << YAML::Key << "Name" << YAML::Value << GetName();
		out << YAML::Key << "Tag" << YAML::Value << GetTag();
		out << YAML::Key << "Active" << YAML::Value << IsActiveSelf();

		m_transform->Serialize(out);

		for (auto& [type, component] : m_components)
		{
			if (type == eComponentType::Transform)
				continue;
			component->Serialize(out);
		}
	}

	void GameObject::Deserialize(const YAML::Node& node)
	{
		if (GetInstanceID() != node["ID"].as<uint64_t>())
		{
			DV_LOG(GameObject, eLogLevel::Err, "[::Deserialize] InstanceID 초기화에 실패하였습니다.");
			//SetInstanceID(node["ID"].as<uint64_t>());
			return;
		}

		SetName(node["Name"].as<std::string>());
		SetTag(node["Tag"].as<std::string>());
		SetActive(node["Active"].as<bool>());

		m_transform->Deserialize(node["Transform"]);
		m_transform->SubscribeEvents();

		if (auto comNode = node["Camera"])
		{
			auto com = AddComponent<Camera>(comNode["ID"].as<uint64_t>());
			com->Deserialize(comNode);
			com->SubscribeEvents();
		}
		if (auto comNode = node["Light"])
		{
			auto com = AddComponent<Light>(comNode["ID"].as<uint64_t>());
			com->Deserialize(comNode);
			com->SubscribeEvents();
		}
		if (auto comNode = node["MeshRenderer"])
		{
			//auto com = AddComponent<MeshRenderer>(comNode["ID"].as<uint64_t>());
			MeshRenderer* com = nullptr;
			if(com = GetComponent<MeshRenderer>())
			{
				com->SetInstanceID(comNode["ID"].as<uint64_t>());
			}
			else
			{
				com = AddComponent<MeshRenderer>(comNode["ID"].as<uint64_t>());
			}

			com->Deserialize(comNode);
			com->SubscribeEvents();
		}
		/*
		if(auto comNode = node["MonoBehaviour"])
		{
			auto com = AddComponent<MonoBehaviour>(comNode["ID"].as<uint64_t>());
			com->Deserialize(comNode);
			com->SubscribeEvents();
		}
		*/
	}

	void GameObject::RemoveComponentByType(eComponentType type)
	{
		if (HasComponentByType(type))
		{
			auto it = m_components.find(type);
			if (it != m_components.end())
			{
				it->second.reset();
				m_components.erase(it);
			}
		}
	}

	bool GameObject::HasComponentByType(eComponentType type) const
	{
		return m_components.find(type) != m_components.end();
	}

	Component* GameObject::GetComponentByType(eComponentType type) const
	{
		auto it = m_components.find(type);
		if (it != m_components.end())
			return it->second.get();

		return nullptr;
	}
	
	void GameObject::SetActive(bool value)
	{
		m_isActiveSelf = value;
		bool parentHierarchy = m_transform->HasParent() ? 
			m_transform->GetParent()->GetGameObject()->m_isActiveHierarchy : true;

		updateActiveInHierarchy(parentHierarchy);
	}

	void GameObject::OnParentChanged()
	{
		m_scene->NotifyParentChanged(this);
	}

	Transform* GameObject::GetTransform() const
	{
		assert(m_transform);
		return m_transform.get();
	}

	GameObject* GameObject::GetParent() const
	{
		auto* parentTransform = m_transform->GetParent();
		return parentTransform ? parentTransform->GetGameObject() : nullptr;
	}

	void GameObject::SetParent(GameObject* parent)
	{
		m_transform->SetParent(parent ? parent->GetTransform() : nullptr);
	}

	GameObject* GameObject::FindChildByName(const std::string& name)
	{
		auto* found = m_transform->Find(name);
		return found ? found->GetGameObject() : nullptr;
	}

	void GameObject::updateActiveInHierarchy(bool parentHierarchy)
	{
		m_isActiveHierarchy = parentHierarchy && m_isActiveSelf;

		// 이렇게 자식들을 전부 변경하는 것 보다
		// 업데이트 할 때 확인하는 편이 나으려나..
		const auto& children = m_transform->GetChildren();
		for (auto child : children)
		{
			auto owner = child->GetGameObject();
			owner->updateActiveInHierarchy(m_isActiveHierarchy);
		}
	}
}
