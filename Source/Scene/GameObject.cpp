#include "pch.h"
#include "GameObject.h"
#include "Components/Transform.h"

namespace Dive
{
	GameObject::GameObject()
		: Object()
	{
		m_transform = std::make_unique<Transform>(this);
	}

	GameObject::~GameObject() = default;

	void GameObject::Update()
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
}