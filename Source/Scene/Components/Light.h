#pragma once
#include <memory>
#include <DirectXMath.h>

#include "Component.h"
#include "Core/Types.h"
#include "Graphics/ConstantBufferDatas.h"

namespace Dive
{
	class GameObject;

	class Light : public Component
	{
	public:
		Light(GameObject* owner);
		virtual ~Light() override = default;

		virtual void Update() override;

		eLightType GetLightType() const { return static_cast<eLightType>(m_data.type); }
		void SetLightType(eLightType type);

		DirectX::XMFLOAT3 GetColor() const { return m_data.color; }
		void SetColor(const DirectX::XMFLOAT3& color);
		void SetColor(float r, float g, float b);

		float GetRange() const;
		void SetRange(float range);

		float GetInnerAngleDegrees() const;
		void SetInnerAngleDegrees(float degree);

		float GetOuterAngleDegrees() const;
		void SetOuterAngleDegrees(float degree);

		float GetIntensity() const { return m_data.intensity; }
		void SetIntensity(float intensity);

		const LightData& GetLightData() const { return m_data; }

		static constexpr eComponentType GetType() { return eComponentType::Light; }

	private:
		LightData m_data{};
	};
}