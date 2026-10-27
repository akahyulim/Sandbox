#include "pch.h"
#include "GameObject.h"
#include "Scene.h"
#include "Components/Transform.h"
#include "Components/Camera.h"
#include "Components/MeshRenderer.h"
#include "Components/Light.h"

namespace Dive
{
	GameObject::GameObject(const std::string& name)
	{
		SetName(name);
		m_transform = std::make_unique<Transform>(this);
	}

	GameObject::GameObject(Scene* scene, uint64_t id, const std::string& name)
		: Object(id),
		m_scene(scene)
	{
		SetName(name);
		m_transform = std::make_unique<Transform>(this);
	}

	GameObject::~GameObject()
	{
		if (m_parent)
			m_parent->RemoveChild(this);
	}

	void GameObject::Update()
	{
		m_transform->Update();

		for (auto& [type, component] : m_components)
			component->Update();

		for (auto child : m_children)
			child->Update();
	}

	Component* GameObject::GetComponentByType(eComponentType type) const
	{
		auto it = m_components.find(type);
		if (it != m_components.end())
			return it->second.get();

		return nullptr;
	}

	void GameObject::SetTag(const std::string& name)
	{
		Tag tag(name);
		if (m_tag != tag)
		{
			m_tag = tag;
			notifySceneChanged();
		}
	}

	bool GameObject::IsActive() const
	{
		if (m_parent)
			return m_isActive && m_parent->IsActive();

		return m_isActive;
	}

	void GameObject::SetActive(bool active)
	{
		if (m_isActive != active)
		{
			m_isActive = active;
			notifySceneChanged();
		}
	}

	void GameObject::SetParent(GameObject* parent)
	{
		if (parent == m_parent)
			return;

		// 자손이 부모가 될 수 없다.
		if (parent && parent->IsDescendantOf(this))
			return;

		if (m_parent)
			DetachFromParent();
		
		m_parent = parent;

		if (m_parent)
			m_parent->m_children.emplace_back(this);

		notifySceneChanged();
	}
	
	void GameObject::DetachFromParent()
	{
		if (m_parent == nullptr)
			return;

		auto& siblings = m_parent->m_children;
		auto it = std::find(siblings.begin(), siblings.end(), this);
		if (it != siblings.end())
			siblings.erase(it);

		m_parent = nullptr;

		notifySceneChanged();
	}

	GameObject* GameObject::GetChildByIndex(uint32_t index) const
	{
		if(m_children.size() <= index)
			return nullptr;

		return m_children[index];
	}

	void GameObject::DetachChildren()
	{
		if (m_children.empty())
			return;

		for (auto child : m_children)
			child->m_parent = nullptr;

		m_children.clear();

		notifySceneChanged();
	}

	// 사실상 단일 자식을 독립시키고 있다.
	// 이름과 매칭이 되지 않아 부자연스럽다.
	// 애초에 detach children과 차이를 모르겠다.
	// detach child로 살려둬야 하나 싶다.
	void GameObject::RemoveChild(GameObject* child)
	{
		if (child == nullptr || child->GetParent() != this)
			return;

		auto it = std::find(m_children.begin(), m_children.end(), child);
		if (it != m_children.end())
			m_children.erase(it);

		notifySceneChanged();
	}

	// this가 target의 자손인지 확인
	bool GameObject::IsDescendantOf(GameObject* target) const
	{
		if(target == nullptr)
			return false;

		GameObject* currentParent = this->m_parent;
		while (currentParent)
		{
			if (currentParent == target)
				return true;

			currentParent = currentParent->m_parent;
		}

		return false;
	}

	void GameObject::GetDecendants(std::vector<GameObject*>& outDecendants) const
	{
		for (auto child : m_children)
		{
			outDecendants.emplace_back(child);

			if (child->HasChildren())
				child->GetDecendants(outDecendants);
		}
	}

	void GameObject::Serialize(YAML::Emitter& out)
	{
		out << YAML::Key << "ID" << YAML::Value << GetInstanceID();
		out << YAML::Key << "Name" << YAML::Value << GetName();
		out << YAML::Key << "Tag" << YAML::Value << GetTag();
		out << YAML::Key << "Active" << YAML::Value << IsActive();

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
		SetName(node["Name"].as<std::string>());
		SetTag(node["Tag"].as<std::string>());
		SetActive(node["Active"].as<bool>());

		m_transform->Deserialize(node["Transform"]);
		//m_transform->SubscribeEvents();

		if (auto comNode = node["Camera"])
		{
			auto com = AddComponent<Camera>();
			com->Deserialize(comNode);
			//com->SubscribeEvents();
		}
		if (auto comNode = node["Light"])
		{
			auto com = AddComponent<Light>();
			com->Deserialize(comNode);
			//com->SubscribeEvents();
		}
		if (auto comNode = node["MeshRenderer"])
		{
			auto com = AddComponent<MeshRenderer>();
			com->Deserialize(comNode);
			//com->SubscribeEvents();
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

	void GameObject::notifySceneChanged()
	{
		if (m_scene)
		{
			m_scene->SetDirty();
		}
	}
}