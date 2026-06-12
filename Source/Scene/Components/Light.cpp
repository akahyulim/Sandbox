#include "pch.h"
#include "Light.h"
#include "Transform.h"

namespace Dive
{
	Light::Light(GameObject* owner)
		: Component(owner)
	{
	}

	void Light::Update()
	{
		Transform* transform = GetTransform();

		m_data.position = transform->GetPosition();
		m_data.direction = transform->GetForward();
	}
}