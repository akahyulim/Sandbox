#include "pch.h"
#include "Light.h"
#include "Transform.h"

namespace Dive
{
	Light::Light(GameObject* owner)
		: Component(owner)
	{
	}

	// 현재로선 필요없다. Set 메서드에서 직접 전달했다.
	void Light::Update()
	{
		Transform* transform = GetTransform();

		//m_data.position = transform->GetPosition();
		//m_data.direction = transform->GetForward();z
	}
}