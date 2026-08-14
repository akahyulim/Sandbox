#include "pch.h"
#include "RenderTexture.h"
#include "Graphics.h"

namespace Dive
{
	RenderTexture::RenderTexture(Graphics* graphics, const D3D11_TEXTURE2D_DESC& desc, D3D11_SUBRESOURCE_DATA* data)
		: m_graphics(graphics)
		, m_desc(desc)
	{
		auto device = graphics->GetDevice();
		auto hr = device->CreateTexture2D(&desc, data, m_texture2D.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("ID3D11Texture2D 생성 실패: {}", ErrorUtils::ToVerbose(hr));
			assert(false && "ID3D11Texture2D 생성 실패!");
		}

		if (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
		{
			createShaderResourceView();
		}
		if (desc.BindFlags & D3D11_BIND_RENDER_TARGET)
		{
			createRenderTargetViews();
		}
		
		if (desc.BindFlags & D3D11_BIND_DEPTH_STENCIL)
		{
			createDepthStencilViews();
		}
	}

	ID3D11RenderTargetView* RenderTexture::GetRenderTargetView(uint32_t index) const
	{
		if (index >= m_renderTargetViews.size())
			return nullptr;

		return m_renderTargetViews[index].Get();
	}

	ID3D11DepthStencilView* RenderTexture::GetDetphStencilView(uint32_t index) const
	{
		if (index >= m_depthStencilViews.size())
			return nullptr;

		return m_depthStencilViews[index].Get();
	}
	
	void RenderTexture::createShaderResourceView()
	{
		auto device = m_graphics->GetDevice();

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = m_desc.Format;

		if (m_desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE)
		{
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MostDetailedMip = 0;
			srvDesc.TextureCube.MipLevels = m_desc.MipLevels;
		}
		else
		{
			if (m_desc.ArraySize > 1)
			{
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
				srvDesc.Texture2DArray.MostDetailedMip = 0;
				srvDesc.Texture2DArray.MipLevels = m_desc.MipLevels;
				srvDesc.Texture2DArray.FirstArraySlice = 0;
				srvDesc.Texture2DArray.ArraySize = m_desc.ArraySize;
			}
			else
			{
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.Texture2D.MipLevels = m_desc.MipLevels;
			}
		}

		auto hr = device->CreateShaderResourceView(
			static_cast<ID3D11Resource*>(m_texture2D.Get()),
			&srvDesc,
			m_shaderResourceView.GetAddressOf());

		if (FAILED(hr))
		{
			spdlog::error("ShaderResourceView 생성 실패: {}", ErrorUtils::ToVerbose(hr));
			assert(false && "ID3D11ShaderResourceView 생성 실패!");
		}
	}

	void RenderTexture::createRenderTargetViews()
	{
		auto device = m_graphics->GetDevice();

		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = m_desc.Format;

		if (m_desc.ArraySize > 1)
		{
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			rtvDesc.Texture2DArray.MipSlice = 0;
			rtvDesc.Texture2DArray.ArraySize = 1;

			m_renderTargetViews.resize(m_desc.ArraySize);
			for (UINT i = 0; i < m_desc.ArraySize; ++i)
			{
				rtvDesc.Texture2DArray.FirstArraySlice = i;

				auto hr = device->CreateRenderTargetView(
					m_texture2D.Get(),
					&rtvDesc,
					m_renderTargetViews[i].GetAddressOf());

				if (FAILED(hr))
				{
					spdlog::error("RenderTargetView (Index: {}) 생성 실패: {}", i, ErrorUtils::ToVerbose(hr));
					assert(false && "ID3D11RenderTargetView 생성 실패!");
				}
			}
		}
		else
		{
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Texture2D.MipSlice = 0;

			m_renderTargetViews.resize(1);
			auto hr = device->CreateRenderTargetView(
				m_texture2D.Get(),
				&rtvDesc,
				m_renderTargetViews[0].GetAddressOf());

			if (FAILED(hr))
			{
				spdlog::error("RenderTargetView 생성 실패: {}", ErrorUtils::ToVerbose(hr));
				assert(false && "ID3D11RenderTargetView 생성 실패!");
			}
		}
	}

	void RenderTexture::createDepthStencilViews()
	{
		auto device = m_graphics->GetDevice();

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = m_desc.Format;

		if (m_desc.ArraySize > 1)
		{
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
			dsvDesc.Texture2DArray.MipSlice = 0;
			dsvDesc.Texture2DArray.ArraySize = 1;

			m_depthStencilViews.resize(m_desc.ArraySize);
			for (UINT i = 0; i < m_desc.ArraySize; ++i)
			{
				dsvDesc.Texture2DArray.FirstArraySlice = i;

				auto hr = device->CreateDepthStencilView(
					m_texture2D.Get(),
					&dsvDesc,
					m_depthStencilViews[i].GetAddressOf());

				if (FAILED(hr))
				{
					spdlog::error("DepthStencilView (Index: {}) 생성 실패: {}", i, ErrorUtils::ToVerbose(hr));
					assert(false && "ID3D11DepthStencilView 생성 실패!");
				}
			}
		}
		else
		{
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Texture2D.MipSlice = 0;

			m_depthStencilViews.resize(1);
			auto hr = device->CreateDepthStencilView(
				m_texture2D.Get(),
				&dsvDesc,
				m_depthStencilViews[0].GetAddressOf());

			if (FAILED(hr))
			{
				spdlog::error("DepthStencilView 생성 실패: {}", ErrorUtils::ToVerbose(hr));
				assert(false && "ID3D11DepthStencilView 생성 실패!");
			}
		}
	}
}