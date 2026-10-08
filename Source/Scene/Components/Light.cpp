#include "pch.h"
#include "Light.h"
#include "Transform.h"
#include "Utilities/Math.h"

namespace Dive
{
	Light::Light(GameObject* owner)
		: Component(owner)
	{
	}

    void Light::Update()
    {
        Transform* transform = GetTransform();
        assert(transform != nullptr);

        DirectX::XMFLOAT3 curPos = transform->GetPosition();
        DirectX::XMFLOAT3 curDir = transform->GetForward();

        if (!Math::XMFLOAT3Equal(m_data.position, curPos) ||
            !Math::XMFLOAT3Equal(m_data.direction, curDir))
        {
            m_data.position = curPos;
            m_data.direction = curDir;
            m_isDirty = true;
        }
    }
    void Light::SetLightType(eLightType type)
    {
        uint32_t index = static_cast<uint32_t>(type);

        if (m_data.type == index)
            return;

        m_data.type = index;
        m_isDirty = true;
    }

    void Light::SetColor(const DirectX::XMFLOAT3& color)
    {
        SetColor(color.x, color.y, color.z);
    }

    void Light::SetColor(float r, float g, float b)
    {
        if (Math::XMFLOAT3Equal(m_data.color, DirectX::XMFLOAT3(r, g, b)))
            return;

        m_data.color = { r, g, b };
        m_isDirty = true;
    }

    float Light::GetRange() const
    {
        return (m_data.rangeRcp > 0.0001f) ?
            1.0f / m_data.rangeRcp : 0.0f;
    }

    void Light::SetRange(float range)
    {
        auto rangeRcp = (range > 0.0001f) ?
            1.0f / range : 0.0f;

        if (fabsf(m_data.rangeRcp - rangeRcp) < 1e-5f)
            return;

        m_data.rangeRcp = rangeRcp;
        m_isDirty = true;
    }

    float Light::GetInnerAngleDegrees() const
    {
        float radian = acosf(m_data.cosInnerAngle);
        return DirectX::XMConvertToDegrees(radian);
    }

    void Light::SetInnerAngleDegrees(float degree)
    {
        degree = std::clamp(degree, 0.0f, 90.0f);
        float cosRadian = cosf(DirectX::XMConvertToRadians(degree));

        if (fabsf(m_data.cosInnerAngle - cosRadian) < 1e-5f)
            return;

        m_data.cosInnerAngle = cosRadian;

        if (m_data.cosInnerAngle < m_data.cosOuterAngle)
            m_data.cosOuterAngle = m_data.cosInnerAngle;

        m_isDirty = true;
    }

    float Light::GetOuterAngleDegrees() const
    {
        float radian = acosf(m_data.cosOuterAngle);
        return DirectX::XMConvertToDegrees(radian);
    }

    void Light::SetOuterAngleDegrees(float degree)
    {
        degree = std::clamp(degree, 0.0f, 90.0f);
        float cosRadian = cosf(DirectX::XMConvertToRadians(degree));

        if (fabsf(m_data.cosOuterAngle - cosRadian) < 1e-5f)
            return;
        
        m_data.cosOuterAngle = cosRadian;

        if (m_data.cosInnerAngle < m_data.cosOuterAngle)
            m_data.cosInnerAngle = m_data.cosOuterAngle;

        m_isDirty = true;
    }

    void Light::SetIntensity(float intensity)
    {
        if (m_data.intensity == intensity)
            return;

        m_data.intensity = intensity;
        m_isDirty = true;
    }
}