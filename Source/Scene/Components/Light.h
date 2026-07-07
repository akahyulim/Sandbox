#pragma once
#include "Component.h"
#include "Graphics/ShaderType.h"
#include "Core/Types.h"

#include <DirectXMath.h>

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

		Color GetColor() const { return Color(m_data.color.x, m_data.color.y, m_data.color.z, 1.0f); }
		void SetColor(const Color& color) { m_data.color = DirectX::XMFLOAT3(color.r, color.g, color.b); }
		void SetColor(float r, float g, float b) { m_data.color = { r, g, b }; }

		DirectX::XMFLOAT3 GetDirection() const { return m_data.direction; }
		void SetDirection(const DirectX::XMFLOAT3& dir) { m_data.direction = dir; }
		void SetDirection(float x, float y, float z) { m_data.direction = { x, y ,z }; }

		const cbLight& GetLightData() const { return m_data; }

		static constexpr eComponentType GetType() { return eComponentType::Light; }

	private:
		cbLight m_data{};
	};
}