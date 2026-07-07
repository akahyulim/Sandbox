#include "pch.h"
#include "GameObject.h"
#include "Scene.h"
#include "Components/Transform.h"

namespace Dive
{
	GameObject::GameObject(uint64_t id)
		: Object(id)
	{
		m_transform = std::make_unique<Transform>(this);
	}

	GameObject::GameObject(uint64_t id, Scene* scene)
		: m_scene(scene)
		, Object(id)
	{
		m_transform = std::make_unique<Transform>(this);
	}

	GameObject::~GameObject() = default;

	void GameObject::Update(float dt)
	{
		m_transform->Update();

		for (auto& [type, com] : m_components)
			com->Update();
	}

	Component* GameObject::GetComponentByType(eComponentType type) const
	{
		auto it = m_components.find(type);
		if (it != m_components.end())
			return it->second.get();

		return nullptr;
	}

	void GameObject::SetParent(GameObject* parent)
	{
		if (parent == m_parent)
			return;

		std::unique_ptr<GameObject> selfOwnership;

		if (m_parent)
			selfOwnership = m_parent->RemoveChild(this);
		else
			m_scene->RemoveRoot(this);

		if (parent)
			parent->AddChild(std::move(selfOwnership));
		else if(m_scene)
			m_scene->AddRoot(std::move(selfOwnership));

	}
	
	void GameObject::DetachFromParent()
	{
		if (m_parent == nullptr)
			return;

		std::unique_ptr<GameObject> self = m_parent->RemoveChild(this);

		if (m_scene && self)
			m_scene->AddRoot(std::move(self));
	}
	
	void GameObject::AddChild(std::unique_ptr<GameObject> child)
	{
		if (child == nullptr || child.get() == this)
			return;

		if (child->m_parent != nullptr)
			child->m_parent->RemoveChild(child.get());

		child->m_parent = this;

		m_children.emplace_back(std::move(child));
	}

	std::unique_ptr<GameObject> GameObject::RemoveChild(GameObject* child)
	{
		auto it = std::find_if(m_children.begin(), m_children.end(),
			[child](const auto& ptr) { return ptr.get() == child; });

		if (it != m_children.end())
		{
			std::unique_ptr<GameObject> detached = std::move(*it);
			detached->m_parent = nullptr;
			m_children.erase(it);
			return detached;
		}

		return nullptr;
	}

	void GameObject::notifySceneChanged() 
	{
		if (m_scene) 
			m_scene->SetDirty();
	}
}