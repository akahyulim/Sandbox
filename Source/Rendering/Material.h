#pragma once
#include <array>
#include <string>
#include <memory>

#include "Resource/Resource.h"
#include "TextureManager.h"
#include "Graphics/ConstantBufferDatas.h"
#include "Graphics/ConstantBuffer.h"

namespace Dive
{
	enum class eMapType : uint8_t
	{
		Albedo,
		Normal,
		ORM,         // Occlusion (R), Roughness (G), Metallic (B) 통합 맵
		Displacement,
		Roughness,
		Metallic,
		Count
	};

	class Graphics;

	class Material : public Resource
	{
	public:
		Material();
		virtual ~Material();

		ID3D11ShaderResourceView* GetMap(eMapType type) const;
		void SetMap(eMapType type, const std::string& path);
		void SetMap(eMapType type, TextureHandle handle);
		bool HasMap(eMapType type) const;

		DirectX::XMFLOAT4 GetBaseColor() const { return m_data.baseColor; }
		void SetBaseColor(const DirectX::XMFLOAT4& color);
		void SetBaseColor(float r, float g, float b, float a);

		DirectX::XMFLOAT3 GetEmissiveFactor() const { return m_data.emissiveFactor; }
		void SetEmissiveFactor(const DirectX::XMFLOAT3& factor);
		void SetEmissiveFactor(float r, float g, float b);

		float GetRoughnessFactor() const { return m_data.roughnessFactor; }
		void SetRoughnessFactor(float factor);

		DirectX::XMFLOAT2 GetTiling() const { return m_data.tiling; }
		void SetTiling(const DirectX::XMFLOAT2& tiling);
		void SetTiling(float x, float y);

		float GetHeightScale() const { return m_data.heightScale; }
		void SetHeightScale(float scale);

		DirectX::XMFLOAT2 GetOffset() const { return m_data.offset; }
		void SetOffset(const DirectX::XMFLOAT2& offset);
		void SetOffset(float x, float y);

		float GetMetallicFactor() const { return m_data.metallicFactor; }
		void SetMetallicFactor(float factor);

		uint32_t GetFlags() const { return m_data.flags; }

		bool IsTransparent() const { return m_isTransparent; }
		void SetTransparent(bool transparent);

		void Bind(Graphics* graphics);

	private:
		MaterialData m_data{};
		std::array<TextureHandle, static_cast<size_t>(eMapType::Count)> m_maps{};

		std::unique_ptr<ConstantBuffer<MaterialData>> m_cbuffer;

		bool m_isTransparent = false;
	};
}
