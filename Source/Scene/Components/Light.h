#pragma once
#include <memory>
#include <DirectXMath.h>

#include "Component.h"
#include "Core/Types.h"
#include "Graphics/ConstantBuffer.h"
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
		void SetLightType(eLightType type) { m_data.type = static_cast<uint32_t>(type); }

		DirectX::XMFLOAT3 GetColor() const { return m_data.color; }
		void SetColor(const DirectX::XMFLOAT3& color) { m_data.color = color; }
		void SetColor(float r, float g, float b) { m_data.color = { r, g, b }; }

		DirectX::XMFLOAT3 GetDirection() const { return m_data.direction; }
		void SetDirection(const DirectX::XMFLOAT3& dir) { m_data.direction = dir; }
		void SetDirection(float x, float y, float z) { m_data.direction = { x, y ,z }; }

		float GetRange() const;
		void SetRange(float range);

		float GetIntensity() const { return m_data.intensity; }
		void SetIntensity(float intensity);

		const LightData& GetLightData() const { return m_data; }

		static constexpr eComponentType GetType() { return eComponentType::Light; }

	private:
		LightData m_data{};
	};
}