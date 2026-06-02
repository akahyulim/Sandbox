#pragma once

#include <cstdint>
#include <DirectXMath.h>
#include <memory>
#include <yaml-cpp/yaml.h>

#include "Component.h"
#include "Rendering/Frustum.h"

namespace Dive
{
    class ConstantBuffer;

    enum class eLightType
    {
        Directional,
        Point,
        Spot
    };

    struct alignas(16) LightData
    {
        uint32_t type;
        DirectX::XMFLOAT3 color;
       
        float intensity;
        DirectX::XMFLOAT3 position;
        
        float rangeRcp;
        DirectX::XMFLOAT3 direction;
        
        float innerAngle;
        float outerAngle;
        DirectX::XMFLOAT2 padding;
    };

	class Light : public Component
	{
	public:
        Light(GameObject* owner, uint64_t id = AUTO_ID);
        ~Light() override = default;

        bool IsEqualTo(const Component* other) const override;

        std::unique_ptr<Component> Clone(GameObject* owner) const override;

        void Serialize(YAML::Emitter& out) override;
        void Deserialize(const YAML::Node& node) override;

        static constexpr eComponentType GetComponentType() { return eComponentType::Light; }

        eLightType GetLightType() const { return m_lightType; }
        void SetLightType(eLightType type);

        DirectX::XMFLOAT3 GetColor() const { return m_color; }
        void SetColor(const DirectX::XMFLOAT3& color);
        void SetColor(float r, float g, float b);

        float GetIntensity() const { return m_intensity; }
        void SetIntensity(float intensity);

        float GetRange() const { return m_range; }
        float GetRangeRcp() const { return 1.0f / m_range; }
        void SetRange(float range);

        float GetInnerAngleRadians() const { return m_innerAngle; }
        float GetInnerAngleDegrees() const { return DirectX::XMConvertToDegrees(m_innerAngle); }
        void SetInnerAngleRadians(float radian);// { m_innerAngle = radian; }
        void SetInnerAngleDegrees(float degree);// { m_innerAngle = DirectX::XMConvertToRadians(degree); }

        float GetOuterAngleRadians() const { return m_outerAngle; }
        float GetOuterAngleDegrees() const { return DirectX::XMConvertToDegrees(m_outerAngle); }
        void SetOuterAngleRandians(float radian);// { m_outerAngle = radian; }
        void SetOuterAngleDegrees(float degree);// { m_outerAngle = DirectX::XMConvertToRadians(degree); }

        DirectX::XMFLOAT3 GetPosition() const;
        DirectX::XMFLOAT3 GetDirection() const;

        LightData GetLightData() const;

        bool IsVisible(const Frustum& frustum) const;

    private:
	private:
        eLightType m_lightType;
        DirectX::XMFLOAT3 m_color;
        float m_intensity;
        float m_range;
        float m_innerAngle;
        float m_outerAngle;
	};
}