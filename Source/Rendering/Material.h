#pragma once
#include <array>
#include <string>

#include "TextureManager.h"

namespace Dive
{
	enum class eMapType : uint8_t
	{
		Albedo,
		Normal,
		Roughness,
		Metallic,
		Emissive,
		Opacity,
		Count
	};

	class Graphics;

	class Material 
	{
	public:
		Material();
		~Material() = default;

		ID3D11ShaderResourceView* GetMap(eMapType type) const;
		void SetMap(const std::string& path, eMapType type);

		DirectX::XMFLOAT4 GetBaseColor() const { return m_baseColor; }
		void SetBaseColor(float r, float g, float b, float a) { m_baseColor = { r, g, b, a }; }

		DirectX::XMFLOAT2 GetTiling() const { return m_tiling; }
		void SetTiling(float x, float y) { m_tiling = { x, y }; }

		DirectX::XMFLOAT2 GetOffset() const { return m_offset; }
		void SetOffset(float x, float y) { m_offset = { x, y }; }

		uint32_t GetFlags() const { return m_flags; }

		void Bind(Graphics* graphics);

	private:
		std::array<TextureHandle, static_cast<size_t>(eMapType::Count)> m_maps{};
		
		DirectX::XMFLOAT4 m_baseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		DirectX::XMFLOAT2 m_tiling = { 1.0f, 1.0f };
		DirectX::XMFLOAT2 m_offset = { 0.0f, 0.0f };

		uint32_t m_flags = 0;
	};
}
