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
    
    float Light::GetRange() const
    {
        // rangeRcp가 0이면 원래 range를 알 수 없으므로 안전하게 처리
        if (m_data.rangeRcp > 0.0001f)
        {
            return 1.0f / m_data.rangeRcp;
        }
        return 0.0f; // 혹은 기본 범위값 반환
    }

    void Light::SetRange(float range)
    {
        // 0이거나 너무 작은 값이 들어와서 생기는 크래시(Inf, NaN) 방지
        if (range > 0.0001f)
        {
            m_data.rangeRcp = 1.0f / range;
        }
        else
        {
            m_data.rangeRcp = 0.0f;
        }
    }

    void Light::SetIntensity(float intensity)
    {
        if (m_data.intensity == intensity)
            return;

        m_data.intensity = intensity;
        m_isDirty = true;   // MarkDirty()로 변경?
    }
}