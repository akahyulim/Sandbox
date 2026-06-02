#include "pch.h"
#include "Texture.h"
#include "Graphics/Graphics.h"

namespace Dive
{
	DvTexture::~DvTexture()
	{
		Release();
	}

	void DvTexture::Release()
	{
		m_shaderResourceView.Reset();
		m_texture.Reset();
	}

	void DvTexture::Bind(eShaderResourceSlot slot)
	{
		if (slot == eShaderResourceSlot::Count)
		{
			spdlog::error("[::Bind] 잘못된 슬롯 지정");
			return;
		}

		assert(m_shaderResourceView);

		//GraphicsDevice::GetDeviceContext()->PSSetShaderResources(static_cast<UINT>(slot), 1, m_shaderResourceView.GetAddressOf());
	}

	uint32_t DvTexture::CalculateMipmapLevels(uint32_t width, uint32_t height)
	{
		return std::max(1u, static_cast<uint32_t>(std::log2(std::max(width, height))) + 1);
	}

	uint32_t DvTexture::GetPixelSize(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_R32_FLOAT:
			return 4;
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			return 8;
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			return 16;
		default:
			spdlog::warn("[::GetPixelSize] 잘못된 포멧 전달");
			return 0;
		}
	}

	bool DvTexture::CanGenerateMips(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24G8_TYPELESS:
			return false;
		default:
			return true;
		}
	}

	bool DvTexture::IsCompressedFormat(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC4_TYPELESS:
		case DXGI_FORMAT_BC4_UNORM:
		case DXGI_FORMAT_BC4_SNORM:
		case DXGI_FORMAT_BC5_TYPELESS:
		case DXGI_FORMAT_BC5_UNORM:
		case DXGI_FORMAT_BC5_SNORM:
		case DXGI_FORMAT_BC6H_TYPELESS:
		case DXGI_FORMAT_BC6H_UF16:
		case DXGI_FORMAT_BC6H_SF16:
		case DXGI_FORMAT_BC7_TYPELESS:
		case DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			return true;
		default:
			return false;
		}
	}

	uint32_t DvTexture::CalculateRowPitch(DXGI_FORMAT format, uint32_t width)
	{
		if (IsCompressedFormat(format))
		{
			// BC1, BC4는 블록당 8바이트 / 나머지는 블록당 16바이트
			uint32_t blockSize = (format == DXGI_FORMAT_BC1_UNORM ||
				format == DXGI_FORMAT_BC1_UNORM_SRGB ||
				format == DXGI_FORMAT_BC4_UNORM ||
				format == DXGI_FORMAT_BC4_SNORM) ? 8 : 16;

			// 가로 방향 블록 개수 계산 (최소 1개)
			uint32_t numBlocksWide = std::max<uint32_t>(1, (width + 3) / 4);
			return numBlocksWide * blockSize;
		}
		else
		{
			// 일반 포맷은 (가로 픽셀 수 * 픽셀당 바이트 수)
			// GetPixelSize는 기존에 가지고 계신 유틸리티를 활용하세요.
			return width * GetPixelSize(format);
		}
	}

	UINT DvTexture::CalculateSlicePitch(DXGI_FORMAT format, UINT width, UINT height)
	{
		// 압축 포맷(BC1~7)인 경우
		if (IsCompressedFormat(format))
		{
			UINT rowPitch = CalculateRowPitch(format, width);
			UINT numRows = std::max<UINT>(1, (height + 3) / 4);
			return rowPitch * numRows;
		}
		// 일반 포맷인 경우
		return CalculateRowPitch(format, width) * height;
	}

	// =============================================================================================================================

	Texture::~Texture()
	{
		Release();
	}

	void Texture::Release()
	{
		m_shaderResourceView.Reset();
		m_texture.Reset();
	}

	void Texture::Bind(eShaderResourceSlot slot)
	{
		if (slot == eShaderResourceSlot::Count)
		{
			spdlog::error("[::Bind] 잘못된 슬롯 지정");
			return;
		}

		assert(m_shaderResourceView);

		//GraphicsDevice::GetDeviceContext()->PSSetShaderResources(static_cast<UINT>(slot), 1, m_shaderResourceView.GetAddressOf());
	}

	uint32_t Texture::CalculateMipmapLevels(uint32_t width, uint32_t height)
	{
		return std::max(1u, static_cast<uint32_t>(std::log2(std::max(width, height))) + 1);
	}

	uint32_t Texture::GetPixelSize(DXGI_FORMAT format)
	{
		switch (format) 
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_R32_FLOAT:
			return 4;
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			return 8;
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			return 16;
		default:
			spdlog::warn("[::GetPixelSize] 잘못된 포멧 전달");
			return 0;
		}
	}

	bool Texture::CanGenerateMips(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24G8_TYPELESS:
			return false;
		default:
			return true;
		}
	}
}
