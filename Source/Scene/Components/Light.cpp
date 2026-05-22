#include "stdafx.h"
#include "Light.h"
#include "Transform.h"
#include "../GameObject.h"
#include "Graphics/GraphicsSystem.h"
#include "Resource/ResourceSystem.h"

namespace Dive
{
	Light::Light(GameObject* owner, uint64_t id)
		: Component(owner, id)
		, m_lightType(eLightType::Directional)
		, m_color(1.0f, 1.0f, 1.0f)
		, m_intensity(0.5f)
		, m_range(50.0f)
		, m_innerAngle(DirectX::XMConvertToRadians(15.0f))
		, m_outerAngle(DirectX::XMConvertToRadians(30.0f))
	{
	}

	bool Light::IsEqualTo(const Component* other) const
	{
		return false;
	}

	std::unique_ptr<Component> Light::Clone(GameObject* owner) const
	{
		auto clone = std::make_unique<Light>(owner);

		clone->m_lightType = m_lightType;
		clone->m_color =  m_color;
		clone->m_intensity = m_intensity;
		clone->m_range = m_range;
		clone->m_innerAngle = m_innerAngle;
		clone->m_outerAngle = m_outerAngle;

		return clone;
	}

	void Light::Serialize(YAML::Emitter& out)
	{
		if (!m_isDirty)
			return;
		
		out << YAML::Key << "Light" << YAML::Value << YAML::BeginMap;
		out << YAML::Value << "ID" << YAML::Value << GetInstanceID();
		out << YAML::Value << "LightType" << YAML::Value << static_cast<int>(GetLightType());
		out << YAML::Value << "LightColor" << YAML::Value << GetColor();
		out << YAML::Value << "LightIntensity" << YAML::Value << GetIntensity();
		out << YAML::Value << "LightRange" << YAML::Value << GetRange();
		out << YAML::EndMap;

		m_isDirty = false;
	}

	void Light::Deserialize(const YAML::Node& node)
	{
		if (GetInstanceID() != node["ID"].as<uint64_t>())
		{
			DV_LOG(GameObject, eLogLevel::Err, "[::Deserialize] InstanceID 초기화에 실패하였습니다.");
			return;
		}
		
		SetLightType(static_cast<eLightType>(node["LightType"].as<int>()));
		SetColor(node["LightColor"].as<DirectX::XMFLOAT3>());
		SetIntensity(node["LightIntensity"].as<float>());
		SetRange(node["LightRange"].as<float>());

		m_isDirty = true;
	}

	void Light::SetLightType(eLightType type)
	{
		if (m_lightType != type)
		{
			m_lightType = type;

			// 유니티 설정 참조
			m_intensity = type == eLightType::Directional ? 0.5f : 1.0f;
		}
	}

	void Light::SetColor(const DirectX::XMFLOAT3& color)
	{
		if (m_color.x != color.x ||
			m_color.y != color.y ||
			m_color.z != color.z)
		{
			m_color = color;
			m_isDirty = true;
		}
	}

	void Light::SetColor(float r, float g, float b)
	{
		SetColor(DirectX::XMFLOAT3(r, g, b));
	}

	void Light::SetIntensity(float intensity)
	{
		if (m_intensity != intensity)
		{
			m_intensity = intensity;
			m_isDirty = true;
		}
	}

	void Light::SetRange(float range)
	{
		if (m_range != range)
		{
			m_range = range;
			m_isDirty = true;
		}
	}

	void Light::SetInnerAngleRadians(float radian)
	{
		if (m_innerAngle != radian)
		{
			m_innerAngle = radian;
			m_isDirty = true;
		}
	}

	void Light::SetInnerAngleDegrees(float degree)
	{
		SetInnerAngleRadians(DirectX::XMConvertToRadians(degree));
	}

	void Light::SetOuterAngleRandians(float radian)
	{
		if (m_outerAngle != radian)
		{
			m_outerAngle = radian;
			m_isDirty = true;
		}
	}

	void Light::SetOuterAngleDegrees(float degree)
	{
		SetOuterAngleRandians(DirectX::XMConvertToRadians(degree));
	}

	DirectX::XMFLOAT3 Light::GetPosition() const
	{
		assert(GetTransform());
		return GetTransform()->GetPosition();
	}
	
	DirectX::XMFLOAT3 Light::GetDirection() const
	{
		assert(GetTransform());
		return GetTransform()->GetLocalForward();
	}

	LightData Light::GetLightData() const
	{
		LightData data{};
		data.type = static_cast<uint32_t>(m_lightType);
		data.color = m_color;
		data.intensity = m_intensity;
		data.position = GetPosition();
		data.direction = GetDirection();
		data.rangeRcp = 1.0f / m_range;
		data.innerAngle = m_innerAngle;
		data.outerAngle = m_outerAngle;

		return data;
	}

	bool Light::IsVisible(const Frustum& frustum) const
	{
		if (eLightType::Directional == m_lightType)
			return true;

		return frustum.CheckSphere(GetPosition(), m_range);
	}
}