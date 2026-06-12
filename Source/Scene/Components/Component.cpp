#include "pch.h"
#include "Component.h"
#include "Transform.h"
#include "Scene/GameObject.h"

namespace Dive
{
	Component::Component(GameObject* owner)
		: m_owner(owner)
	{
	}


	GameObject* Component::GetOwner() const
	{
		return m_owner;
	}

	Transform* Component::GetTransform() const
	{
		return m_owner != nullptr ? m_owner->GetTransform() : nullptr;
	}
}