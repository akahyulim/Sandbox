#include "pch.h"
#include "GameObject.h"
//#include "Scene.h"
#include "Components/Transform.h"

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

	void GameObject::Update(float dt)
	{
		m_transform->Update();

		for (auto& [type, component] : m_components)
			component->Update();

		for (auto child : m_children)
			child->Update(dt);
	}

	Component* GameObject::GetComponentByType(eComponentType type) const
	{
		auto it = m_components.find(type);
		if (it != m_components.end())
			return it->second.get();

		return nullptr;
	}

	bool GameObject::IsActive() const
	{
		if (m_parent)
			return m_isActive && m_parent->IsActive();

		return m_isActive;
	}

	void GameObject::SetParent(GameObject* parent)
	{
		if (parent == m_parent)
			return;

		// 자손이 부모가 될 수 없다.
		if (IsDescendantOf(parent))
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

		auto it = std::find(m_parent->m_children.begin(), m_parent->m_children.end(), this);
		if (it != m_parent->m_children.end())
			m_parent->m_children.erase(it);

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

	void GameObject::notifySceneChanged()
	{
		if (m_scene)
		{
			m_scene->SetDirty();
		}
	}
}