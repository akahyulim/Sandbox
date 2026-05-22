#include "pch.h"
#include "Graphics.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "Shader/Shader.h"
#include "Shader/InputLayout.h"


namespace Dive
{
	namespace
	{
		constexpr UINT DV_BUFFER_COUNT = 2;
		constexpr UINT DV_REFRESHRATE_NUMERATOR = 60;
		constexpr UINT DV_REFRESHRATE_DENOMINATOR = 1;
		constexpr DXGI_FORMAT DV_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
		constexpr BOOL DV_WINDOWED = TRUE;

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

		std::vector<D3D11_INPUT_ELEMENT_DESC> GetInputElements(eInputLayout type)
		{
			std::vector<D3D11_INPUT_ELEMENT_DESC> elements;

			switch (type)
			{

			case eInputLayout::Unlit:
				elements.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				elements.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				break;
			case eInputLayout::Lit:
				elements.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				elements.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				elements.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				elements.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				elements.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				break;
			case eInputLayout::Skinned:
				elements.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				elements.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				elements.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				elements.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				elements.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				elements.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				elements.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				break;
			default:
				spdlog::error("잘못된 입력 레이아웃 타입 전달");
				break;
			}

			return elements;
		}
	}

	Graphics::~Graphics() = default;

	bool Graphics::Initialize(HWND hWnd, uint32_t width, uint32_t height, bool windowed)
	{
		assert(hWnd);
		assert(width != 0 && height != 0);

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
			spdlog::error("그래픽스 디바이스 생성 실패");
			return false;
		}

		IDXGIDevice* dxgiDevice{};
		m_device->QueryInterface(IID_IDXGIDevice, (void**)&dxgiDevice);
		IDXGIAdapter* dxgiAdapter{};
		dxgiDevice->GetParent(IID_IDXGIAdapter, (void**)&dxgiAdapter);
		IDXGIFactory* dxgiFactory{};
		dxgiAdapter->GetParent(IID_IDXGIFactory, (void**)&dxgiFactory);

		DXGI_SWAP_CHAIN_DESC desc{};
		desc.BufferCount = DV_BUFFER_COUNT;
		desc.BufferDesc.Width = width;
		desc.BufferDesc.Height = height;
		desc.BufferDesc.Format = DV_FORMAT;
		desc.BufferDesc.RefreshRate.Denominator = DV_REFRESHRATE_DENOMINATOR;
		desc.BufferDesc.RefreshRate.Numerator = DV_REFRESHRATE_NUMERATOR;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.SampleDesc.Count = 1;								// 멀티 샘플링 off
		desc.SampleDesc.Quality = 0;
		desc.Windowed = windowed;
		desc.OutputWindow = hWnd;
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	// rastertek에선 0이고 다른 값들 설정이 남아 있다...

		hr = dxgiFactory->CreateSwapChain(m_device.Get(), &desc, m_swapChain.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("그래픽스 스왑체인 생성 실패");
			return false;
		}

		DV_RELEASE(dxgiFactory);
		DV_RELEASE(dxgiAdapter);
		DV_RELEASE(dxgiDevice);


		if (!setupViews())
			return false;
		if (!createDepthStencilStates())
			return false;
		if (!createRasterizerStates())
			return false;
		if (!createBlendStates())
			return false;
		if (!createSamplerStates())
			return false;

		OnResizeViews();

		return true;
	}

	// 윈도우 모드에서는 크기,
	// 전체화면 모드에서는 해상도를 갱신
	// 후 wm_size 발생
	void Graphics::Resize(uint32_t  width, uint32_t height)
	{
		assert(m_swapChain);

		DXGI_MODE_DESC desc{};
		desc.Width = static_cast<UINT>(width);
		desc.Height = static_cast<UINT>(height);
		desc.RefreshRate.Numerator = DV_REFRESHRATE_NUMERATOR;
		desc.RefreshRate.Denominator = DV_REFRESHRATE_DENOMINATOR;
		desc.Format = DV_FORMAT;

		HRESULT hr = m_swapChain->ResizeTarget(&desc);
		if (FAILED(hr))
		{
			spdlog::error("크기 재설정 실패");
			return;
		}
	}

	void Graphics::OnResizeViews()
	{
		if (!resizeSwapChain())
			return;

		if (!setupViews())
			return;

		spdlog::info("바뀐 크기: {} x {}", m_width, m_height);
	}

	void Graphics::SetPipelineState(const PipelineState& state)
	{
		ID3D11VertexShader* vs = state.vertexShader ? state.vertexShader->GetShader() : nullptr;
		ID3D11PixelShader* ps = state.pixelShader ? state.pixelShader->GetShader() : nullptr;
		ID3D11InputLayout* il = state.inputLayout ? state.inputLayout->GetLayout() : nullptr;

		ID3D11DepthStencilState* dss = state.depthStencilState != eDepthStencilState::Count
			? m_depthStencilStates[static_cast<size_t>(state.depthStencilState)].get() : nullptr;
		ID3D11RasterizerState* rs = state.rasterizerState != eRasterizerState::Count
			? m_rasterizerStates[static_cast<size_t>(state.rasterizerState)].get() : nullptr;
		ID3D11BlendState* bs = state.blendState != eBlendState::Count
			? m_blendStates[static_cast<size_t>(state.blendState)].get() : nullptr;
		ID3D11SamplerState* ss = state.samplerState != eSamplerState::Count
			? m_samplerStates[static_cast<size_t>(state.samplerState)].get() : nullptr;

		bindVertexShader(vs);
		bindPixelShader(ps);
		bindInputLayout(il);
		bindPrimitiveTopology(state.topology);

		bindDepthStencilState(dss, state.stencilRef);
		bindRasterizerState(rs);
		bindBlendState(bs, state.blendFactor, state.sampleMask);
		bindSamplerState(0, ss);
	}

	void Graphics::BeginRenderPass(const RenderPass& pass)
	{
		assert(m_deviceContext);

		m_deviceContext->OMSetRenderTargets(pass.count, pass.rtvs, pass.dsv);

		m_currentRTVCount = pass.count;
		for (uint32_t i = 0; i < 8; ++i)
		{
			m_currentRTVs[i] = (i < pass.count) ? pass.rtvs[i] : nullptr;
		}
		m_currentDSV = pass.dsv;


		if (pass.colorLoadOp == eLoadOp::Clear)
		{
			for (uint32_t i = 0; i < m_currentRTVCount; ++i)
			{
				if (m_currentRTVs[i])
					m_deviceContext->ClearRenderTargetView(m_currentRTVs[i], &pass.clearColor.r);
			}
		}

		if (pass.depthLoadOp == eLoadOp::Clear && m_currentDSV != nullptr)
		{
			m_deviceContext->ClearDepthStencilView(
				m_currentDSV,
				D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
				pass.clearDepth,
				pass.clearStencil
			);
		}

		/*
		if (!(m_currentViewport == pass.viewport))
		{
			D3D11_VIEWPORT dxViewport{};
			dxViewport.TopLeftX = pass.viewport.topLeftX;
			dxViewport.TopLeftY = pass.viewport.topLeftY;
			dxViewport.Width = pass.viewport.width;
			dxViewport.Height = pass.viewport.height;
			dxViewport.MinDepth = 0.0f;
			dxViewport.MaxDepth = 1.0f;

			m_deviceContext->RSSetViewports(1, &dxViewport);
			m_currentViewport = pass.viewport;
		}
		*/
	}

	void Graphics::EndRenderPass()
	{
		assert(m_deviceContext);

		ID3D11RenderTargetView* nullRTVs[8] = { nullptr };
		m_deviceContext->OMSetRenderTargets(8, nullRTVs, nullptr);

		m_currentRTVCount = 0;
		for (int i = 0; i < 8; ++i)
		{
			m_currentRTVs[i] = nullptr;
		}
		m_currentDSV = nullptr;
	}

	void Graphics::BindMainRenderTarget()
	{
		assert(m_deviceContext);
		m_deviceContext->OMSetRenderTargets(1, m_backbufferRTV.GetAddressOf(), m_backbufferDSV.Get());

		m_currentRTVCount = 1;
		m_currentRTVs[0] = m_backbufferRTV.Get();
		m_currentDSV = m_backbufferDSV.Get();
	}

	void Graphics::Draw(uint32_t vertexCount, uint32_t startVertexLocation)
	{
		assert(m_deviceContext);
		m_deviceContext->Draw(vertexCount, startVertexLocation);
	}

	void Graphics::DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
	{
		assert(m_deviceContext);
		m_deviceContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
	}

	void Graphics::Present() const
	{
		assert(m_swapChain);
		m_swapChain->Present(m_vSync ? 1 : 0, 0);
	}

	std::unique_ptr<VertexBuffer> Graphics::CreateVertexBuffer(uint32_t stride, uint32_t count, const void* data)
	{
		auto vb = std::make_unique<VertexBuffer>(stride, count);

		D3D11_BUFFER_DESC desc{};
		desc.ByteWidth = static_cast<UINT>(stride * count);
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.MiscFlags = 0;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA subData{};
		subData.pSysMem = data;

		if (FAILED(m_device->CreateBuffer(&desc, &subData, vb->GetAddressOf())))
		{
			spdlog::error("정점 버퍼 생성 실패");
			return nullptr;
		}

		return vb;
	}

	std::unique_ptr<IndexBuffer> Graphics::CreateIndexBuffer(eFormat format, uint32_t count, const void* data)
	{
		auto ib = std::make_unique<IndexBuffer>(format, count);

		D3D11_BUFFER_DESC desc{};
		desc.ByteWidth = static_cast<UINT>(ib->GetStride() * ib->GetCount());
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		desc.MiscFlags = 0;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA subData{};
		subData.pSysMem = data;

		if (FAILED(m_device->CreateBuffer(&desc, &subData, ib->GetAddressOf())))
		{
			spdlog::error("인덱스 버퍼 생성 실패");
			return nullptr;
		}

		return ib;
	}

	std::shared_ptr<VertexShader> Graphics::CreateVertexShader(const void* byteCode, size_t size)
	{
		auto vs = std::make_shared<VertexShader>();

		if (FAILED(m_device->CreateVertexShader(
			byteCode,
			size,
			nullptr,
			vs->GetAddressOf()
		)))
		{
			spdlog::error("정점 셰이더 생성 실패");
			return nullptr;
		}

		return vs;
	}

	std::shared_ptr<PixelShader> Graphics::CreatePixelShader(const void* byteCode, size_t size)
	{
		auto ps = std::make_shared<PixelShader>();

		if (FAILED(m_device->CreatePixelShader(
			byteCode,
			size,
			nullptr,
			ps->GetAddressOf()
		)))
		{
			spdlog::error("픽셀 셰이더 생성 실패");
			return nullptr;
		}

		return ps;
	}

	std::shared_ptr<InputLayout> Graphics::CreateInputLayout(eInputLayout type, const void* byteCode, size_t size)
	{
		auto il = std::make_shared<InputLayout>();

		auto element = GetInputElements(type);

		if (FAILED(m_device->CreateInputLayout(
			element.data(),
			static_cast<UINT>(element.size()),
			byteCode,
			size,
			il->GetAddressOf()
		)))
		{
			spdlog::error("인풋 레이아웃 샐성 실패");
			return nullptr;
		}

		return il;
	}

	void Graphics::BindVertexBuffer(VertexBuffer* vb)
	{
		assert(m_deviceContext);

		if (m_currentVB == vb)
			return;

		if (vb)
		{
			ID3D11Buffer* rawBuffer = vb->GetRawBuffer();
			uint32_t stride = vb->GetStride();
			uint32_t offset = 0;

			m_deviceContext->IASetVertexBuffers(0, 1, &rawBuffer, &stride, &offset);
		}
		else
		{
			ID3D11Buffer* nullBuffer = nullptr;
			uint32_t zero = 0;
			m_deviceContext->IASetVertexBuffers(0, 1, &nullBuffer, &zero, &zero);
		}

		m_currentVB = vb;
	}

	void Graphics::BindIndexBuffer(IndexBuffer* ib)
	{
		assert(m_deviceContext);

		if (m_currentIB == ib)
			return;

		if (ib)
		{
			DXGI_FORMAT format = ConvertToDXGIFormat(ib->GetFormat());
			ID3D11Buffer* rawBuffer = ib->GetRawBuffer();

			m_deviceContext->IASetIndexBuffer(
				ib->GetRawBuffer(),
				ConvertToDXGIFormat(ib->GetFormat()),
				0
			);
		}
		else
		{
			m_deviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
		}

		m_currentIB = ib;
	}

	void Graphics::BindVSConstantBuffer(eCBufferSlotVS slot, ConstantBuffer* cb)
	{
		assert(m_deviceContext);

		uint32_t slotIdx = static_cast<uint32_t>(slot);
		ID3D11Buffer* rawBuffer = cb ? cb->GetRawBuffer() : nullptr;

		if (m_currentVSCB[slotIdx] == rawBuffer)
			return;

		m_currentVSCB[slotIdx] = rawBuffer;
		m_deviceContext->VSSetConstantBuffers(slotIdx, 1, &rawBuffer);
	}

	void Graphics::BindPSConstantBuffer(eCBufferSlotPS slot, ConstantBuffer* cb)
	{
		assert(m_deviceContext);

		uint32_t slotIdx = static_cast<uint32_t>(slot);
		ID3D11Buffer* rawBuffer = cb ? cb->GetRawBuffer() : nullptr;

		if (m_currentPSCB[slotIdx] == rawBuffer)
			return;

		m_currentPSCB[slotIdx] = rawBuffer;
		m_deviceContext->PSSetConstantBuffers(slotIdx, 1, &rawBuffer);
	}

	bool Graphics::setupViews()
	{
		assert(m_swapChain.Get());
		assert(m_device.Get());

		m_backbufferRTV.Reset();
		m_backbufferTexture.Reset();
		m_backbufferDSV.Reset();

		ID3D11Texture2D* backBuffer = nullptr;
		HRESULT hr = m_swapChain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&backBuffer);
		if (FAILED(hr))
		{
			spdlog::error("백버퍼 획득 실패");
			return false;
		}

		hr = m_device->CreateRenderTargetView(static_cast<ID3D11Resource*>(backBuffer), nullptr, m_backbufferRTV.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("백버퍼의 렌더타겟 뷰 생성 실패");
			return false;
		}

		// 반드시 릴리즈!!!
		backBuffer->Release();

		DXGI_SWAP_CHAIN_DESC desc{};
		m_swapChain->GetDesc(&desc);

		D3D11_TEXTURE2D_DESC texDesc{};
		texDesc.Width = desc.BufferDesc.Width;
		texDesc.Height = desc.BufferDesc.Height;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		hr = m_device->CreateTexture2D(&texDesc, nullptr, m_backbufferTexture.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("백버퍼의 깊이 스텐실 버퍼 생성 실패");
			return false;
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc{};
		viewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipSlice = 0;

		hr = m_device->CreateDepthStencilView(static_cast<ID3D11Resource*>(m_backbufferTexture.Get()), &viewDesc, m_backbufferDSV.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("백버퍼의 깊이 스텐실 뷰 생성 실패");
			return false;
		}

		m_width = static_cast<uint32_t>(desc.BufferDesc.Width);
		m_height = static_cast<uint32_t>(desc.BufferDesc.Height);

		return true;
	}

	bool Graphics::createDepthStencilStates()
	{
		return false;
	}

	bool Graphics::createRasterizerStates()
	{
		return false;
	}

	bool Graphics::createBlendStates()
	{
		return false;
	}

	bool Graphics::createSamplerStates()
	{
		return false;
	}

	bool Graphics::resizeSwapChain()
	{
		assert(m_swapChain);
		assert(m_deviceContext);

		ID3D11RenderTargetView* nullRTV[1] = { nullptr };
		m_deviceContext->OMSetRenderTargets(1, nullRTV, nullptr);
		m_backbufferRTV.Reset();

		HRESULT hr = m_swapChain->ResizeBuffers(0, 0, 0, DV_FORMAT, 0);
		if (FAILED(hr))
		{
			spdlog::error("백버퍼 크기 재설정 실패");
			return false;
		}

		return true;
	}

	void Graphics::bindVertexShader(ID3D11VertexShader* vs)
	{
		assert(m_deviceContext);

		if (m_currentVS == vs)
			return;

		m_deviceContext->VSSetShader(vs, nullptr, 0);
		m_currentVS = vs;
	}

	void Graphics::bindPixelShader(ID3D11PixelShader* ps)
	{
		assert(m_deviceContext);

		if (m_currentPS == ps)
			return;

		m_deviceContext->PSSetShader(ps, nullptr, 0);
		m_currentPS = ps;
	}

	void Graphics::bindInputLayout(ID3D11InputLayout* il)
	{
		assert(m_deviceContext);

		if (m_currentLayout == il)
			return;

		m_deviceContext->IASetInputLayout(il);
		m_currentLayout = il;
	}

	void Graphics::bindPrimitiveTopology(ePrimitiveTopology topology)
	{
		assert(m_deviceContext);

		if (m_currentTopology == topology)
			return;

		m_deviceContext->IASetPrimitiveTopology(ConvertToDXTopology(topology));
		m_currentTopology = topology;
	}

	void Graphics::bindDepthStencilState(ID3D11DepthStencilState* dss, uint32_t stencilRef)
	{
		assert(m_deviceContext);

		if (m_currentDSS == dss && m_currentStencilRef == stencilRef)
			return;

		m_deviceContext->OMSetDepthStencilState(dss, stencilRef);
		m_currentDSS = dss;
		m_currentStencilRef = stencilRef;
	}

	void Graphics::bindRasterizerState(ID3D11RasterizerState* rs)
	{
		assert(m_deviceContext);

		if (m_currentRS == rs)
			return;

		m_deviceContext->RSSetState(rs);
		m_currentRS = rs;
	}

	void Graphics::bindBlendState(ID3D11BlendState* bs, const float* blendFactor, uint32_t sampleMask)
	{
		assert(m_deviceContext);

		float defaultFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		const float* targetFactor = blendFactor ? blendFactor : defaultFactor;

		bool isFactorEqual = (m_currentBlendFactor[0] == targetFactor[0]) &&
			(m_currentBlendFactor[1] == targetFactor[1]) &&
			(m_currentBlendFactor[2] == targetFactor[2]) &&
			(m_currentBlendFactor[3] == targetFactor[3]);

		if (m_currentBS == bs && m_currentSampleMask == sampleMask && isFactorEqual)
			return;

		m_deviceContext->OMSetBlendState(bs, blendFactor, sampleMask);

		m_currentBS = bs;
		m_currentSampleMask = sampleMask;

		m_currentBlendFactor[0] = targetFactor[0];
		m_currentBlendFactor[1] = targetFactor[1];
		m_currentBlendFactor[2] = targetFactor[2];
		m_currentBlendFactor[3] = targetFactor[3];
	}

	void Graphics::bindSamplerState(uint32_t slot, ID3D11SamplerState* ss)
	{
		assert(m_deviceContext);
		assert(slot < 16);

		if (m_currentSamplers[slot] == ss)
			return;

		m_deviceContext->VSSetSamplers(slot, 1, &ss);
		m_deviceContext->PSSetSamplers(slot, 1, &ss);

		m_currentSamplers[slot] = ss;
	}
}
