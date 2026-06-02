#include "stdafx.h"
#include "RenderTexture.h"
//#include "Core/CoreSystem.h"
#include "Graphics/GraphicsSystem.h"

namespace Dive
{
	DvRenderTexture::DvRenderTexture(uint32_t width, uint32_t height, eDepthFormat depth, DXGI_FORMAT format, bool useMips)
	{
		m_info.width = width;
		m_info.height = height;
		m_info.format = format;
		m_info.useMips = useMips;

		m_depthFormat = depth;		// 이건 어떻게 하는 게 좋으려나...

		SetName("DvRenderTexture");
	}

	DvRenderTexture::~DvRenderTexture()
	{
		Release();
	}

	bool DvRenderTexture::Serialize(std::ofstream& fout) const
	{
		DV_LOG(DvRenderTexture, eLogLevel::Warn, "[::Serialize] 미구현");
		return true;
	}

	bool DvRenderTexture::Deserialize(std::ifstream& fin)
	{
		DV_LOG(DvRenderTexture, eLogLevel::Warn, "[::Deserialize] 미구현");
		return true;
	}

	bool DvRenderTexture::Create()
	{
		if (m_info.format == DXGI_FORMAT_UNKNOWN)
		{
			DV_LOG(DvRenderTexture, eLogLevel::Err, "[::Create] 잘못된 포멧 설정");
			return false;
		}

		m_info.mipLevels = CanGenerateMips(m_info.format) ? (m_info.useMips ? CalculateMipmapLevels(m_info.width, m_info.height) : 1) : 1;

		D3D11_TEXTURE2D_DESC texDesc{};
		texDesc.Format = m_info.format;
		texDesc.Width = static_cast<UINT>(m_info.width);
		texDesc.Height = static_cast<UINT>(m_info.height);
		texDesc.MipLevels = static_cast<UINT>(m_info.mipLevels);
		texDesc.ArraySize = 1;
		texDesc.SampleDesc.Count = 1;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texDesc.MiscFlags = m_info.useMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

		auto hr = GraphicsDevice::GetDevice()->CreateTexture2D(&texDesc, nullptr, m_texture.GetAddressOf());
		if (FAILED(hr))
		{
			DV_LOG(DvRenderTexture, eLogLevel::Err, "[::Create] CreateTexture2D 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
		rtvDesc.Format = m_info.format;

		hr = GraphicsDevice::GetDevice()->CreateRenderTargetView(static_cast<ID3D11Resource*>(m_texture.Get()), &rtvDesc, m_renderTargetView.GetAddressOf());
		if (FAILED(hr))
		{
			DV_LOG(DvRenderTexture, eLogLevel::Err, "[::Create] CreateRenderTargetView 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC dsvDesc{};
		dsvDesc.Format = m_info.format;
		dsvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipLevels = m_info.mipLevels;
		dsvDesc.Texture2D.MostDetailedMip = 0;

		hr = GraphicsDevice::GetDevice()->CreateShaderResourceView(static_cast<ID3D11Resource*>(m_texture.Get()), &dsvDesc, m_shaderResourceView.GetAddressOf());
		if (FAILED(hr))
		{
			DV_LOG(DvRenderTexture, eLogLevel::Err, "[::Create] CreateShaderResourceView 실패 : {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		if (m_info.useMips && m_shaderResourceView)	GraphicsDevice::GetDeviceContext()->GenerateMips(m_shaderResourceView.Get());

		if (m_depthFormat != eDepthFormat::None)
		{
			D3D11_TEXTURE2D_DESC texDesc{};
			texDesc.Format = (m_depthFormat == eDepthFormat::Depth16) ? DXGI_FORMAT_R16_TYPELESS : DXGI_FORMAT_R24G8_TYPELESS;
			texDesc.Width = static_cast<UINT>(m_info.width);
			texDesc.Height = static_cast<UINT>(m_info.height);
			texDesc.MipLevels = 1;
			texDesc.ArraySize = 1;
			texDesc.SampleDesc.Count = 1;
			texDesc.Usage = D3D11_USAGE_DEFAULT;
			texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

			hr = GraphicsDevice::GetDevice()->CreateTexture2D(&texDesc, nullptr, m_depthTexture.GetAddressOf());
			if (FAILED(hr))
			{
				DV_LOG(DvRenderTexture, eLogLevel::Err, "[::Create] CreateTexture2D 실패 : {}", ErrorUtils::ToVerbose(hr));
				return false;
			}

			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
			dsvDesc.Format = (m_depthFormat == eDepthFormat::Depth16) ? DXGI_FORMAT_D16_UNORM : DXGI_FORMAT_D24_UNORM_S8_UINT;
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Texture2D.MipSlice = 0;

			hr = GraphicsDevice::GetDevice()->CreateDepthStencilView(static_cast<ID3D11Resource*>(m_depthTexture.Get()), &dsvDesc, m_depthDSV.GetAddressOf());
			if (FAILED(hr))
			{
				DV_LOG(DvRenderTexture, eLogLevel::Err, "[::Create] CreateDepthSteniclView 실패 : {}", ErrorUtils::ToVerbose(hr));
				return false;
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = (m_depthFormat == eDepthFormat::Depth16) ? DXGI_FORMAT_R16_UNORM : DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.MostDetailedMip = 0;

			hr = GraphicsDevice::GetDevice()->CreateShaderResourceView(static_cast<ID3D11Resource*>(m_depthTexture.Get()), &srvDesc, m_depthSRV.GetAddressOf());
			if (FAILED(hr))
			{
				DV_LOG(DvRenderTexture, eLogLevel::Err, "[::Create] CreateShaderResourceView 실패 : {}", ErrorUtils::ToVerbose(hr));
				return false;
			}
		}

		return true;
	}

	bool DvRenderTexture::Resize(uint32_t width, uint32_t height)
	{
		m_info.width = width;
		m_info.height = height;

		return Create();
	}

	void DvRenderTexture::Release()
	{
		m_depthSRV.Reset();
		m_depthDSV.Reset();
		m_depthTexture.Reset();
		m_renderTargetView.Reset();
	}

	// ===================================================================================================================================

	RenderTexture::RenderTexture(uint32_t width, uint32_t height, eDepthFormat depth, DXGI_FORMAT format, bool useMips)
	{
		m_width = width;
		m_height = height;
		m_format = format;
		m_depthFormat = depth;
		m_useMips = useMips;

		SetName("RenderTexture");
	}

	RenderTexture::~RenderTexture()
	{
		Release();
	}

	bool RenderTexture::SaveToFile(const std::filesystem::path& filepath)
	{
		DV_LOG(RenderTexture, eLogLevel::Warn, "[::SaveToFile] 미구현");
		return true;
	}

	bool RenderTexture::LoadFromFile(const std::filesystem::path& filepath)
	{
		DV_LOG(RenderTexture, eLogLevel::Warn, "[::LoadFromFile] 미구현");
		return true;
	}

	bool RenderTexture::Serialize(std::ofstream& fout) const
	{
		DV_LOG(RenderTexture, eLogLevel::Warn, "[::Serialize] 미구현");
		return true;
	}

	bool RenderTexture::Deserialize(std::ifstream& fin)
	{
		DV_LOG(RenderTexture, eLogLevel::Warn, "[::Deserialize] 미구현");
		return true;
	}

	bool RenderTexture::Create()
	{
		if (m_format == DXGI_FORMAT_UNKNOWN)
		{
			DV_LOG(RenderTexture, eLogLevel::Err, "[::Create] 잘못된 포멧 설정");
			return false;
		}

		m_mipLevels = CanGenerateMips(m_format) ? (m_useMips ? CalculateMipmapLevels(m_width, m_height) : 1) : 1;

		D3D11_TEXTURE2D_DESC texDesc{};
		texDesc.Format = m_format;
		texDesc.Width = static_cast<UINT>(m_width);
		texDesc.Height = static_cast<UINT>(m_height);
		texDesc.MipLevels = static_cast<UINT>(m_mipLevels);
		texDesc.ArraySize = 1;
		texDesc.SampleDesc.Count = 1;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texDesc.MiscFlags = m_useMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

		auto hr = GraphicsDevice::GetDevice()->CreateTexture2D(&texDesc, nullptr, m_texture.GetAddressOf());
		if (FAILED(hr))
		{
			DV_LOG(RenderTexture, eLogLevel::Err, "[::Create] CreateTexture2D 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
		rtvDesc.Format = m_format;

		hr = GraphicsDevice::GetDevice()->CreateRenderTargetView(static_cast<ID3D11Resource*>(m_texture.Get()), &rtvDesc, m_renderTargetView.GetAddressOf());
		if(FAILED(hr))
		{
			DV_LOG(RenderTexture, eLogLevel::Err, "[::Create] CreateRenderTargetView 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC dsvDesc{};
		dsvDesc.Format = m_format;
		dsvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipLevels = m_mipLevels;
		dsvDesc.Texture2D.MostDetailedMip = 0;

		hr = GraphicsDevice::GetDevice()->CreateShaderResourceView(static_cast<ID3D11Resource*>(m_texture.Get()), &dsvDesc, m_shaderResourceView.GetAddressOf());
		if(FAILED(hr))
		{
			DV_LOG(RenderTexture, eLogLevel::Err, "[::Create] CreateShaderResourceView 실패 : {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		if (m_useMips && m_shaderResourceView)	GraphicsDevice::GetDeviceContext()->GenerateMips(m_shaderResourceView.Get());
		
		if(m_depthFormat != eDepthFormat::None)
		{
			D3D11_TEXTURE2D_DESC texDesc{};
			texDesc.Format = (m_depthFormat == eDepthFormat::Depth16) ? DXGI_FORMAT_R16_TYPELESS : DXGI_FORMAT_R24G8_TYPELESS;
			texDesc.Width = static_cast<UINT>(m_width);
			texDesc.Height = static_cast<UINT>(m_height);
			texDesc.MipLevels = 1;
			texDesc.ArraySize = 1;
			texDesc.SampleDesc.Count = 1;
			texDesc.Usage = D3D11_USAGE_DEFAULT;
			texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

			hr = GraphicsDevice::GetDevice()->CreateTexture2D(&texDesc, nullptr, m_depthTexture.GetAddressOf());
			if(FAILED(hr))
			{
				DV_LOG(RenderTexture, eLogLevel::Err, "[::Create] CreateTexture2D 실패 : {}", ErrorUtils::ToVerbose(hr));
				return false;
			}

			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
			dsvDesc.Format = (m_depthFormat == eDepthFormat::Depth16) ? DXGI_FORMAT_D16_UNORM : DXGI_FORMAT_D24_UNORM_S8_UINT;
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Texture2D.MipSlice = 0;

			hr = GraphicsDevice::GetDevice()->CreateDepthStencilView(static_cast<ID3D11Resource*>(m_depthTexture.Get()), &dsvDesc, m_depthDSV.GetAddressOf());
			if(FAILED(hr))
			{
				DV_LOG(RenderTexture, eLogLevel::Err, "[::Create] CreateDepthSteniclView 실패 : {}", ErrorUtils::ToVerbose(hr));
				return false;
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = (m_depthFormat == eDepthFormat::Depth16) ? DXGI_FORMAT_R16_UNORM : DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.MostDetailedMip = 0;

			hr = GraphicsDevice::GetDevice()->CreateShaderResourceView(static_cast<ID3D11Resource*>(m_depthTexture.Get()), &srvDesc, m_depthSRV.GetAddressOf());
			if(FAILED(hr))
			{
				DV_LOG(RenderTexture, eLogLevel::Err, "[::Create] CreateShaderResourceView 실패 : {}", ErrorUtils::ToVerbose(hr));
				return false;
			}
		}

		return true;
	}

	bool RenderTexture::Resize(uint32_t width, uint32_t height)
	{
		m_width = width;
		m_height = height;
		
		return Create();
	}

	void RenderTexture::Release()
	{
		m_depthSRV.Reset();
		m_depthDSV.Reset();
		m_depthTexture.Reset();
		m_renderTargetView.Reset();
	}
}
