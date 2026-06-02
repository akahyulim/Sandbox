#pragma once
#include <cstdint>
#include <dxgi.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "Resource.h"

namespace Dive
{
	enum class eShaderResourceSlot : uint8_t
	{
		// 🔹 머티리얼 텍스처 슬롯 (Geometry Pass)
		Diffuse = 0,
		NormalMap = 1,
		RoughnessMap = 2,
		MetallicMap = 3,
		EmissiveMap = 4,
		OpacityMap = 5,

		// 🔸 G-Buffer 결과물 슬롯 (Lighting Pass)
		GBuffer_Albedo = 10,
		GBuffer_Normal = 11,
		GBuffer_RoughMetal = 12,
		GBuffer_Emissive = 13,
		GBuffer_Depth = 14,
		GBuffer_MaterialID = 15,

		// 🔸 조명/쉐도우/환경맵
		ShadowMap = 20,
		SkyMap = 21,
		EnvironmentMap = 22,
		IBL_Irradiance = 23,
		IBL_Specular = 24,
		BRDF_LUT = 25,

		// 🔸 후처리용
		SceneColor = 30,
		SceneDepth = 31,
		BloomTexture = 32,
		MotionVector = 33,

		// ✅ 마지막 슬롯 수 확인용
		Count
	};

	class DvTexture : public Resource
	{
	public:
		struct BaseInfo
		{
			DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
			//DXGI_FORMAT depthFormat = DXGI_FORMAT_UNKNOWN;
			uint32_t width = 1;
			uint32_t height = 1;
			bool useMips = false;
			uint32_t mipLevels = 1;
			uint32_t rowPitch = 1;
		};

	public:
		DvTexture() = default;
		~DvTexture() override;

		void Release() override;

		void Bind(eShaderResourceSlot slot);

		uint32_t GetWidth() const { return m_info.width; }
		void SetWidth(uint32_t width) { m_info.width = width; }

		uint32_t GetHeight() const { return m_info.height; }
		void SetHeight(uint32_t height) { m_info.height = height; }

		DXGI_FORMAT GetFormat() const { return m_info.format; }
		void SetFormat(DXGI_FORMAT format) { m_info.format = format; }

		bool IsGenerateMips() const { return m_info.useMips; }
		void SetUseMips(bool use) { m_info.useMips = use; }

		uint32_t GetMipLevels() const { return m_info.mipLevels; }
		void SetMipLevels(uint32_t mipLev) { m_info.mipLevels = mipLev; }

		uint32_t GetRowPitch() const { return m_info.rowPitch; }
		void SetRowPitch(uint32_t rowPitch) { m_info.rowPitch = rowPitch; }

		BaseInfo& GetInfo() { return m_info; }

		ID3D11ShaderResourceView* GetShaderResourceView() const { return m_shaderResourceView.Get(); }

		static uint32_t CalculateMipmapLevels(uint32_t width, uint32_t height);
		static uint32_t GetPixelSize(DXGI_FORMAT format);
		static bool CanGenerateMips(DXGI_FORMAT format);
		static bool IsCompressedFormat(DXGI_FORMAT format);
		static uint32_t CalculateRowPitch(DXGI_FORMAT format, uint32_t width);
		static UINT CalculateSlicePitch(DXGI_FORMAT format, UINT width, UINT height);

	protected:
		virtual std::string GetDefaultExtension() const { return ".dds"; }

	protected:
		BaseInfo m_info;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
	};

	class Texture : public Resource
	{
	public:
		Texture() = default;
		~Texture() override;

		void Release() override;

		void Bind(eShaderResourceSlot slot);

		uint32_t GetWidth() const { return m_width; }
		uint32_t GetHeight() const { return m_height; }

		DXGI_FORMAT GetFormat() const { return m_format; }
		bool IsGenerateMips() const { return m_useMips; }
		uint32_t GetMipLevels() const { return m_mipLevels; }

		ID3D11ShaderResourceView* GetShaderResourceView() const { return m_shaderResourceView.Get(); }

		static uint32_t CalculateMipmapLevels(uint32_t width, uint32_t height);
		static uint32_t GetPixelSize(DXGI_FORMAT format);
		static bool CanGenerateMips(DXGI_FORMAT format);

	protected:
		uint32_t m_width = 1;
		uint32_t m_height = 1;

		DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;
		
		uint32_t m_mipLevels = 1;
		bool m_useMips = false;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
	};
}
