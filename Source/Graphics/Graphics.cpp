#include "pch.h"
#include "Graphics.h"
#include "ConstantBuffer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "InputLayout.h"
#include "Core/Window.h"

namespace Dive
{
	namespace
	{
		constexpr UINT DV_BACKBUFFER_COUNT = 3;
		constexpr UINT DV_REFRESHRATE_NUMERATOR = 60;
		constexpr UINT DV_REFRESHRATE_DENOMINATOR = 1;
		constexpr DXGI_FORMAT DV_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;

		D3D11_PRIMITIVE_TOPOLOGY ConvertToDXTopology(ePrimitiveTopology topology)
		{
			switch (topology)
			{
			case ePrimitiveTopology::PointList:		return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
			case ePrimitiveTopology::LineList:		return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
			case ePrimitiveTopology::LineStrip:		return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
			case ePrimitiveTopology::TriangleList:	return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			case ePrimitiveTopology::TriangleStrip:	return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			default:								return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
			}
		}

		DXGI_FORMAT ConvertToDXGIFormat(eFormat format)
		{
			switch (format)
			{
			case eFormat::R16_UINT:					return DXGI_FORMAT_R16_UINT;
			case eFormat::R32_UINT:					return DXGI_FORMAT_R32_UINT;
			case eFormat::R32_FLOAT:				return DXGI_FORMAT_R32_FLOAT;
			case eFormat::R32G32_FLOAT:				return DXGI_FORMAT_R32G32_FLOAT;
			case eFormat::R32G32B32_FLOAT:			return DXGI_FORMAT_R32G32B32_FLOAT;
			case eFormat::R8G8B8A8_UNORM:			return DXGI_FORMAT_R8G8B8A8_UNORM;
			case eFormat::R8G8B8A8_UNORM_SRGB:		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			case eFormat::D24_UNORM_S8_UINT:		return DXGI_FORMAT_D24_UNORM_S8_UINT;
			default:								return DXGI_FORMAT_UNKNOWN;
			}
		}
	}

	Graphics::Graphics(Window* window)
		: m_window(window)
	{
		m_width = window->GetWidth();
		m_height = window->GetHeight();

		HRESULT hr = ::D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			0,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			m_device.GetAddressOf(),
			nullptr,
			m_deviceContext.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("Graphics::Graphics - 그래픽스 디바이스 생성 실패");
			return;
		}

		IDXGIDevice* dxgiDevice{};
		m_device->QueryInterface(IID_IDXGIDevice, (void**)&dxgiDevice);
		IDXGIAdapter* dxgiAdapter{};
		dxgiDevice->GetParent(IID_IDXGIAdapter, (void**)&dxgiAdapter);
		IDXGIFactory* dxgiFactory{};
		dxgiAdapter->GetParent(IID_IDXGIFactory, (void**)&dxgiFactory);

		DXGI_SWAP_CHAIN_DESC desc{};
		desc.BufferDesc.Width = 0;
		desc.BufferDesc.Height = 0;
		desc.BufferDesc.Format = DV_FORMAT;
		desc.BufferDesc.RefreshRate.Numerator = 0;
		desc.BufferDesc.RefreshRate.Denominator = 0;
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = DV_BACKBUFFER_COUNT;
		desc.OutputWindow = window->GetWindowHandle();
		desc.Windowed = TRUE;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

		hr = dxgiFactory->CreateSwapChain(m_device.Get(), &desc, m_swapChain.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("그래픽스 스왑체인 생성 실패");
			return;
		}

		DV_RELEASE(dxgiFactory);
		DV_RELEASE(dxgiAdapter);
		DV_RELEASE(dxgiDevice);

		createBackbufferResources(m_width, m_height);
	}

	Graphics::~Graphics() = default;

	void Graphics::SetBackbuffer()
	{
		SetViewport(m_width, m_height);
		m_deviceContext->OMSetRenderTargets(1, m_backbufferRTV.GetAddressOf(), nullptr);
	}

	void Graphics::ClearBackbuffer()
	{
		// Begin
		// => m_current들을 전부 nullptr로 초기화한다.
		// 매 프레임 한 번은 초기화하고, 내부 pass 과정에서 참조하는 듯하다.
		{
			m_currentVS = nullptr;
			m_currentPS = nullptr;

			m_currentIL = nullptr;

			m_currentDSS = nullptr;
			m_currentRS = nullptr;
			m_currentBS = nullptr;

			m_currentTopology = ePrimitiveTopology::None;
		}

		float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		m_deviceContext->ClearRenderTargetView(m_backbufferRTV.Get(), clearColor);
	}

	void Graphics::ResizeBackbuffer(uint32_t width, uint32_t height)
	{
		if ((m_width != width || m_height != height)
			&& width > 0 && height > 0)
		{
			m_width = width;
			m_height = height;

			createBackbufferResources(width, height);
		}
	}

	void Graphics::SwapBuffers(bool vSync)
	{
		m_swapChain->Present(vSync ? 1 : 0, 0);
	}

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> Graphics::CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& desc)
	{
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> state;

		auto hr = m_device->CreateDepthStencilState(&desc, state.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("DepthStencilState 생성 실패: {}", ErrorUtils::ToVerbose(hr));
			return nullptr;
		}

		return state;
	}

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> Graphics::CreateRasterizerState(const D3D11_RASTERIZER_DESC& desc)
	{
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> state;

		auto hr = m_device->CreateRasterizerState(&desc, state.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("RasterizerState 생성 실패: {}", ErrorUtils::ToVerbose(hr));
			return nullptr;
		}

		return state;
	}

	Microsoft::WRL::ComPtr<ID3D11BlendState> Graphics::CreateBlendState(const D3D11_BLEND_DESC& desc)
	{
		Microsoft::WRL::ComPtr<ID3D11BlendState> state;

		auto hr = m_device->CreateBlendState(&desc, state.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("BlendState 생성 실패: {}", ErrorUtils::ToVerbose(hr));
			return nullptr;
		}

		return state;
	}

	Microsoft::WRL::ComPtr<ID3D11SamplerState> Graphics::CreateSamplerState(const D3D11_SAMPLER_DESC& desc)
	{
		Microsoft::WRL::ComPtr<ID3D11SamplerState> state;

		auto hr = m_device->CreateSamplerState(&desc, state.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("SamplerState 생성 실패: {}", ErrorUtils::ToVerbose(hr));
			return nullptr;
		}

		return state;
	}

	void Graphics::UpdateBuffer(ID3D11Buffer* cbuffer, const void* data, uint32_t size)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource = {};

		auto hr = m_deviceContext->Map(
			static_cast<ID3D11Resource*>(cbuffer),
			0,
			D3D11_MAP_WRITE_DISCARD,
			0,
			&mappedResource
		);

		if (FAILED(hr))
		{
			spdlog::error("상수버퍼 맵 실패: {}", ErrorUtils::ToVerbose(hr));
			return;
		}

		memcpy(mappedResource.pData, data, size);

		m_deviceContext->Unmap(static_cast<ID3D11Resource*>(cbuffer), 0);
	}

	void Graphics::SetVertexBuffer(VertexBuffer* vb, uint32_t slot)
	{
		ID3D11Buffer* rawBuffer = vb ? vb->GetRawBuffer() : nullptr;
		UINT stride = vb ? static_cast<UINT>(vb->GetStride()) : 0;
		UINT offset = 0;

		m_deviceContext->IASetVertexBuffers(slot, rawBuffer == nullptr ? 0 : 1, &rawBuffer, &stride, &offset);
	}

	void Graphics::SetVertexBuffers(uint32_t startSlot, std::span<VertexBuffer*> vbs)
	{
		if (vbs.empty())
			return;

		constexpr uint32_t MaxBuffers = 8;
		uint32_t count = static_cast<uint32_t>(vbs.size());

		assert(count <= MaxBuffers && "Too many vertex buffers to bind at once!");

		ID3D11Buffer* rawBuffers[MaxBuffers];
		UINT strides[MaxBuffers];
		UINT offsets[MaxBuffers];

		for (uint32_t i = 0; i < count; ++i)
		{
			VertexBuffer* vb = vbs[i];
			if (vb)
			{
				rawBuffers[i] = vb->GetRawBuffer();
				strides[i] = static_cast<UINT>(vb->GetStride());
				offsets[i] = 0;
			}
			else
			{
				rawBuffers[i] = nullptr;
				strides[i] = 0;
				offsets[i] = 0;
			}
		}

		m_deviceContext->IASetVertexBuffers(startSlot, count, rawBuffers, strides, offsets);
	}

	void Graphics::SetIndexBuffer(IndexBuffer* ib)
	{
		ID3D11Buffer* rawBuffer = ib ? ib->GetRawBuffer() : nullptr;
		DXGI_FORMAT format = ib ? ConvertToDXGIFormat(ib->GetFormat()) : DXGI_FORMAT_UNKNOWN;

		m_deviceContext->IASetIndexBuffer(rawBuffer, format, 0);
	}

	void Graphics::SetConstantBuffer(eShaderStage stage, uint32_t slot, ID3D11Buffer* const* ppBuffer)
	{
		switch (stage)
		{
		case eShaderStage::VS:
			m_deviceContext->VSSetConstantBuffers(slot, 1, ppBuffer);
			break;
		case eShaderStage::PS:
			m_deviceContext->PSSetConstantBuffers(slot, 1, ppBuffer);
			break;
		case eShaderStage::HS:
			m_deviceContext->HSSetConstantBuffers(slot, 1, ppBuffer);
			break;
		case eShaderStage::DS:
			m_deviceContext->DSSetConstantBuffers(slot, 1, ppBuffer);
			break;
		case eShaderStage::GS:
			m_deviceContext->GSSetConstantBuffers(slot, 1, ppBuffer);
			break;
		case eShaderStage::CS:
			m_deviceContext->CSSetConstantBuffers(slot, 1, ppBuffer);
			break;
		}
	}

	void Graphics::SetDepthStencilState(ID3D11DepthStencilState* dss, uint32_t stencilRef)
	{
		if (m_currentDSS != dss)
		{
			m_currentDSS = dss;
			m_deviceContext->OMSetDepthStencilState(dss ? m_currentDSS : nullptr, stencilRef);
		}
	}

	void Graphics::SetRasterizerState(ID3D11RasterizerState* rs)
	{
		if (m_currentRS != rs)
		{
			m_currentRS = rs;
			m_deviceContext->RSSetState(rs ? m_currentRS : nullptr);
		}
	}

	void Graphics::SetBlendState(ID3D11BlendState* bs, float* blendFactor, uint32_t smaplerMask)
	{
		if (m_currentBS != bs)
		{
			m_currentBS = bs;
			m_deviceContext->OMSetBlendState(bs ? m_currentBS : nullptr, blendFactor, smaplerMask);
		}
	}

	void Graphics::SetVertexShader(VertexShader* vs)
	{
		if (vs != m_currentVS)
		{
			m_currentVS = vs;
			m_deviceContext->VSSetShader(vs ? *vs : nullptr, nullptr, 0);
		}
	}

	void Graphics::SetPixelShader(PixelShader* ps)
	{
		if (ps != m_currentPS)
		{
			m_currentPS = ps;
			m_deviceContext->PSSetShader(ps ? *ps : nullptr, nullptr, 0);
		}
	}

	void Graphics::SetComputeShader(ComputeShader* cs)
	{
		if (cs != m_currentCS)
		{
			m_currentCS = cs;
			m_deviceContext->CSSetShader(cs ? *cs: nullptr, nullptr, 0);
		}
	}

	void Graphics::SetInputLayout(InputLayout* il)
	{
		if (il != m_currentIL)
		{
			m_currentIL = il;
			m_deviceContext->IASetInputLayout(*il);
		}
	}

	// 임시
	// 1. 단일로 제한
	// 2. ppSrv로 전달
	// 3. switch - case로 구현
	// Views로 할 때 배열로 전달하고 std::span으로 받으면 배열의 사이즈와 데이터를 분리할 수 있다.
	void Graphics::SetShaderResourceView(eShaderStage stage, uint32_t slot, ID3D11ShaderResourceView * const* ppSrv)
	{
		ID3D11ShaderResourceView* nullSrv = nullptr;

		switch (stage)
		{
		case eShaderStage::VS:
			m_deviceContext->VSSetShaderResources(slot, 1, ppSrv != nullptr ? ppSrv : &nullSrv);
			break;
		case eShaderStage::PS:
			m_deviceContext->PSSetShaderResources(slot, 1, ppSrv != nullptr ? ppSrv : &nullSrv);
			break;
		case eShaderStage::CS:
			m_deviceContext->CSSetShaderResources(slot, 1, ppSrv != nullptr ? ppSrv : &nullSrv);
			break;
		}
	}

	void Graphics::SetTopology(ePrimitiveTopology topology)
	{
		if (m_currentTopology != topology)
		{
			m_currentTopology = topology;
			m_deviceContext->IASetPrimitiveTopology(ConvertToDXTopology(m_currentTopology));
		}
	}

	void Graphics::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		D3D11_VIEWPORT vp{};
		vp.TopLeftX = static_cast<FLOAT>(x);
		vp.TopLeftY = static_cast<FLOAT>(y);
		vp.Width = static_cast<FLOAT>(width);
		vp.Height = static_cast<FLOAT>(height);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;

		m_deviceContext->RSSetViewports(1, &vp);
	}

	void Graphics::SetViewport(const Viewport& vp)
	{
		D3D11_VIEWPORT d3d11_vp{};
		d3d11_vp.TopLeftX = vp.topLeftX;
		d3d11_vp.TopLeftY = vp.topLeftY;
		d3d11_vp.Width = vp.width;
		d3d11_vp.Height = vp.height;
		d3d11_vp.MinDepth = vp.minDepth;
		d3d11_vp.MaxDepth = vp.maxDepth;

		m_deviceContext->RSSetViewports(1, &d3d11_vp);
	}

	void Graphics::SetViewport(uint32_t width, uint32_t height)
	{
		D3D11_VIEWPORT vp{};
		vp.TopLeftX = static_cast<FLOAT>(0.0f);
		vp.TopLeftY = static_cast<FLOAT>(0.0f);
		vp.Width = static_cast<FLOAT>(width);
		vp.Height = static_cast<FLOAT>(height);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;

		m_deviceContext->RSSetViewports(1, &vp);
	}

	void Graphics::BeginRenderPass(const RenderPassDesc& desc)
	{
		std::vector<ID3D11RenderTargetView*> rtvs;
		for (const auto& rtDesc : desc.renderTargetDescs)
		{
			rtvs.push_back(rtDesc.RenderTargetView);

			if (rtDesc.AccessType == eLoadAccessOp::Clear)
			{
				m_deviceContext->ClearRenderTargetView(
					rtDesc.RenderTargetView,
					rtDesc.ClearColor
				);
			}
		}

		ID3D11DepthStencilView* dsv = nullptr;
		if (desc.depthStencilDesc.has_value())
		{
			dsv = desc.depthStencilDesc->DepthStencilView;

			if (desc.depthStencilDesc->AccessType == eLoadAccessOp::Clear)
			{
				m_deviceContext->ClearDepthStencilView(
					dsv,
					desc.depthStencilDesc->ClearFlags,
					desc.depthStencilDesc->Depth,
					desc.depthStencilDesc->Stencil
				);
			}
		}

		m_deviceContext->OMSetRenderTargets(
			static_cast<UINT>(rtvs.size()),
			rtvs.data(),
			dsv
		);
	}

	void Graphics::EndRenderPass()
	{
		m_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	}

	void Graphics::Draw(uint32_t vertexCount, uint32_t startVertexLocation)
	{
		m_deviceContext->Draw(vertexCount, startVertexLocation);
	}

	void Graphics::DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
	{
		m_deviceContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
	}

	void Graphics::createBackbufferResources(uint32_t width, uint32_t height)
	{
		m_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);

		if (m_backbufferRTV)
			m_backbufferRTV.Reset();

		m_swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
		m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backBuffer.GetAddressOf());

		m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_backbufferRTV.GetAddressOf());

		m_deviceContext->OMSetRenderTargets(1, m_backbufferRTV.GetAddressOf(), nullptr);
	}
}