#include "pch.h"
#include "Component.h"
#include "Transform.h"
#include "Scene/GameObject.h"

namespace Dive
{
	Component::Component(GameObject* gameObject)
		: m_owner(gameObject)
	{
	}


	GameObject* Component::GetGameObject() const
	{
		return m_owner;
	}

	Transform* Component::GetTransform() const
	{
		return m_owner != nullptr ? m_owner->GetTransform() : nullptr;
	}
}