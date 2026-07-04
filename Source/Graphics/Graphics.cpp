#include "pch.h"
#include "Graphics.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Resource/Shader.h"
#include "Resource/InputLayout.h"
#include "Resource/ShaderProgram.h"
#include "Resource/Texture.h"
#include "Resource/Texture2D.h"
#include "Resource/RenderTexture.h"
#include "Resource/StaticMesh.h"

#include <DirectXTex/DirectXTex.h>

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

	Graphics::Graphics()
	{
	}

	Graphics::~Graphics()
	{
	}

	// 그냥 Window를 전달?
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
			spdlog::error("Graphics::Initialize - 그래픽스 디바이스 생성 실패");
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

		if (!updateBackbuffer())
			return false;
		if (!createDepthStencilStates())
			return false;
		if (!createRasterizerStates())
			return false;
		if (!createBlendStates())
			return false;
		if (!createSamplerStates())
			return false;
		if (!createConstantBuffers())
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

		if (!updateBackbuffer())
			return;

		spdlog::info("바뀐 크기: {} x {}", m_width, m_height);
	}

	void Graphics::SetPipelineState(const PipelineState& state)
	{
		bindPrimitiveTopology(state.topology);

		if (state.shaderProgram)
		{
			bindVertexShader(state.shaderProgram->GetVertexShader());
			bindPixelShader(state.shaderProgram->GetPixelShader());
			bindInputLayout(state.shaderProgram->GetInputLayout());
		}
		else
		{
			bindVertexShader(nullptr);
			bindPixelShader(nullptr);
			bindInputLayout(nullptr);
		}

		ID3D11DepthStencilState* dss = state.depthStencilState != eDepthStencilState::Count
			? m_depthStencilStates[static_cast<size_t>(state.depthStencilState)].Get() : nullptr;
		bindDepthStencilState(dss, state.stencilRef);

		ID3D11RasterizerState* rs = state.rasterizerState != eRasterizerState::Count
			? m_rasterizerStates[static_cast<size_t>(state.rasterizerState)].Get() : nullptr;
		bindRasterizerState(rs);

		ID3D11BlendState* bs = state.blendState != eBlendState::Count
			? m_blendStates[static_cast<size_t>(state.blendState)].Get() : nullptr;
		bindBlendState(bs, state.blendFactor, state.sampleMask);
	}

	void Graphics::BindPipelineState(const PipelineState& pso, uint32_t stencilRef, float blendFactor[4], uint32_t sampleMask)
	{
		assert(m_deviceContext);

		m_deviceContext->IASetPrimitiveTopology(ConvertToDXTopology(pso.topology));

		m_deviceContext->VSSetShader(
			pso.shaderProgram ? pso.shaderProgram->GetVertexShader() : nullptr, 
			nullptr, 0);
		m_deviceContext->PSSetShader(
			pso.shaderProgram ? pso.shaderProgram->GetPixelShader() : nullptr,
			nullptr, 0);
		m_deviceContext->IASetInputLayout(pso.shaderProgram ? pso.shaderProgram->GetInputLayout() : nullptr);

		ID3D11DepthStencilState* dss = pso.depthStencilState != eDepthStencilState::Count
			? m_depthStencilStates[static_cast<size_t>(pso.depthStencilState)].Get() : nullptr;
		m_deviceContext->OMSetDepthStencilState(dss, stencilRef);

		ID3D11RasterizerState* rs = pso.rasterizerState != eRasterizerState::Count
			? m_rasterizerStates[static_cast<size_t>(pso.rasterizerState)].Get() : nullptr;
		m_deviceContext->RSSetState(rs);

		ID3D11BlendState* bs = pso.blendState != eBlendState::Count
			? m_blendStates[static_cast<size_t>(pso.blendState)].Get() : nullptr;
		m_deviceContext->OMSetBlendState(bs, blendFactor, sampleMask);
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

		if (!(m_currentViewport == pass.viewport))
		{
			D3D11_VIEWPORT viewport{};
			viewport.TopLeftX = pass.viewport.topLeftX;
			viewport.TopLeftY = pass.viewport.topLeftY;
			viewport.Width = pass.viewport.width;
			viewport.Height = pass.viewport.height;
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;

			m_deviceContext->RSSetViewports(1, &viewport);
			m_currentViewport = pass.viewport;
		}
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

	// adria의 GfxDevice::SetBackbuffer()와 동일하다.
	void Graphics::BindMainRenderTarget()
	{
		assert(m_deviceContext);
		// dsv는 제외했다.
		m_deviceContext->OMSetRenderTargets(1, m_backbufferRTV.GetAddressOf(), nullptr);

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

	std::shared_ptr<StaticMesh> Graphics::CreateStaticMesh(const StaticGeometryData& data)
	{
		auto mesh = std::make_shared<StaticMesh>();

		// VertexBuffer
		auto& vertices = data.vertices;
		uint32_t stride = static_cast<uint32_t>(sizeof(StaticVertex));
		uint32_t count = static_cast<uint32_t>(vertices.size());

		mesh->m_vertexBuffer = std::move(CreateVertexBuffer(stride, count, vertices.data()));
		if (mesh->m_vertexBuffer == nullptr)
		{
			spdlog::error("Graphics::CreateStaticMesh - VertexBuffer 생성 중 오류 발생");
			return nullptr;
		}

		// IndexBuffer
		auto& indices = data.indices;
		if (!indices.empty())
		{
			bool use32bit = std::any_of(indices.begin(), indices.end(), [](uint32_t i) { return i > 65535; });
			eFormat format = use32bit ? eFormat::R32_UINT : eFormat::R16_UINT;

			std::vector<uint16_t> indices16;
			const void* indexData = nullptr;

			if (use32bit)
			{
				indexData = indices.data();
			}
			else
			{
				indices16.reserve(indices.size());
				for (uint32_t i : indices)
					indices16.push_back(static_cast<uint16_t>(i));
				indexData = indices16.data();
			}
				
			uint32_t count = static_cast<uint32_t>(indices.size());

			mesh->m_indexBuffer = CreateIndexBuffer(format, count, indexData);
			if (mesh->m_indexBuffer == nullptr)
			{
				spdlog::error("Graphics::CreateStaticMesh - IndexBuffer 생성 중 오류 발생");
				return nullptr;
			}
		}

		// SetBound
		/*
		{
			XMFLOAT3 min = { FLT_MAX, FLT_MAX, FLT_MAX };
			XMFLOAT3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

			for (const auto& vertex : m_vertices)
			{
				const XMFLOAT3& pos = vertex.Position;

				min.x = std::min(min.x, pos.x);
				min.y = std::min(min.y, pos.y);
				min.z = std::min(min.z, pos.z);

				max.x = std::max(max.x, pos.x);
				max.y = std::max(max.y, pos.y);
				max.z = std::max(max.z, pos.z);
			}

			m_bounds.center = {
				(min.x + max.x) * 0.5f,
				(min.y + max.y) * 0.5f,
				(min.z + max.z) * 0.5f
			};

			m_bounds.extents = {
				(max.x - min.x) * 0.5f,
				(max.y - min.y) * 0.5f,
				(max.z - min.z) * 0.5f
			};
		}
		*/

		return mesh;
	}

	bool Graphics::CreateRawBuffer(const D3D11_BUFFER_DESC& desc, const D3D11_SUBRESOURCE_DATA* initialData, ID3D11Buffer** ppBuffer)
	{
		assert(m_device);
		assert(ppBuffer);

		auto hr = m_device->CreateBuffer(&desc, initialData, ppBuffer);
		if(FAILED(hr))
		{
			spdlog::error("Graphics::CreateBuffer 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}
		
		return true;
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

		if (!CreateRawBuffer(desc, &subData, vb->GetAddressOf()))
		{
			spdlog::error("Graphics::VertexBuffer - 버퍼 생성 실패");
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

		if (!CreateRawBuffer(desc, &subData, ib->GetAddressOf()))
		{
			spdlog::error("Graphics::CreateIndexBuffer - 버퍼 생성 실패");
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
			spdlog::error("Graphics::CreateVertexShader - 정점 셰이더 생성 실패");
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

	ID3D11VertexShader* Graphics::CreateVS(const void* byteCode, size_t size)
	{
		assert(m_device);

		ID3D11VertexShader* vs = nullptr;

		if (FAILED(m_device->CreateVertexShader(
			byteCode,
			size,
			nullptr,
			&vs
		)))
		{
			spdlog::error("Graphics::CreateVertexShader - 정점 셰이더 생성 실패");
			return nullptr;
		}

		return vs;
	}

	ID3D11InputLayout* Graphics::CreateIL(eInputLayout type, const void* byteCode, size_t size)
	{
		assert(m_device);

		ID3D11InputLayout* il = nullptr;

		auto element = GetInputElements(type);

		if (FAILED(m_device->CreateInputLayout(
			element.data(),
			static_cast<UINT>(element.size()),
			byteCode,
			size,
			&il
		)))
		{
			spdlog::error("Graphics::CreateInputLayout - 인풋 레이아웃 샐성 실패");
			return nullptr;
		}

		return il;
	}

	ID3D11PixelShader* Graphics::CreatePS(const void* byteCode, size_t size)
	{
		assert(m_device);

		ID3D11PixelShader* ps = nullptr;

		if (FAILED(m_device->CreatePixelShader(
			byteCode,
			size,
			nullptr,
			&ps
		)))
		{
			spdlog::error("Graphics::CreatePixelShader - 픽셀 셰이더 생성 실패");
			return nullptr;
		}

		return ps;
	}

	std::shared_ptr<Texture2D> Graphics::CreateTexture2D(DirectX::ScratchImage* scratchImage, DirectX::TexMetadata* metaData)
	{
		assert(m_device);
		assert(scratchImage);
		assert(metaData);

		std::shared_ptr<Texture2D> texture2D = std::make_shared<Texture2D>(
			static_cast<uint32_t>(metaData->width), 
			static_cast<uint32_t>(metaData->height));

		HRESULT hr = DirectX::CreateShaderResourceView(
			m_device.Get(),
			scratchImage->GetImages(),
			scratchImage->GetImageCount(),
			*metaData,
			texture2D->m_srv.GetAddressOf());

		if (FAILED(hr))
		{
			spdlog::error("Graphics::CreateTexture2DFromMemory - CreateShaderResourceView 실패: {}", ErrorUtils::ToVerbose(hr));
			return nullptr;
		}

		return texture2D;
	}

	std::shared_ptr<RenderTexture> Graphics::CreateRenderTexture(uint32_t width, uint32_t height, DXGI_FORMAT colorFormat, DXGI_FORMAT depthFormat)
	{
		assert(m_device);

		std::shared_ptr<RenderTexture> renderTexture = std::make_shared<RenderTexture>(width, height, colorFormat, depthFormat);

		Microsoft::WRL::ComPtr<ID3D11Texture2D> colorTexture;
		D3D11_TEXTURE2D_DESC colorDesc{};
		colorDesc.Width = width;
		colorDesc.Height = height;
		colorDesc.MipLevels = 1;
		colorDesc.ArraySize = 1;
		colorDesc.Format = colorFormat;
		colorDesc.SampleDesc.Count = 1;
		colorDesc.SampleDesc.Quality = 0;
		colorDesc.Usage = D3D11_USAGE_DEFAULT;
		colorDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		colorDesc.CPUAccessFlags = 0;
		colorDesc.MiscFlags = 0;

		HRESULT hr = m_device->CreateTexture2D(&colorDesc, nullptr, colorTexture.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("Graphics::CreateRenderTexture - ColorTexture 생성 실패", ErrorUtils::ToVerbose(hr));
			return nullptr;
		}

		hr = m_device->CreateRenderTargetView(colorTexture.Get(), nullptr, renderTexture->m_rtv.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("Graphics::CreateRenderTexture - RenderTargetView 생성 실패", ErrorUtils::ToVerbose(hr));
			return nullptr;
		}

		hr = m_device->CreateShaderResourceView(colorTexture.Get(), nullptr, renderTexture->m_srv.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("Graphics::CreateRenderTexture - ShaderResourceView 생성 실패", ErrorUtils::ToVerbose(hr));
			return nullptr;
		}

		D3D11_TEXTURE2D_DESC depthDesc{};
		depthDesc.Width = width;
		depthDesc.Height = height;
		depthDesc.MipLevels = 1;
		depthDesc.ArraySize = 1;
		depthDesc.Format = depthFormat;
		depthDesc.SampleDesc.Count = 1;
		depthDesc.SampleDesc.Quality = 0;
		depthDesc.Usage = D3D11_USAGE_DEFAULT;
		depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthDesc.CPUAccessFlags = 0;
		depthDesc.MiscFlags = 0;

		renderTexture->m_depthTexture.Reset();

		hr = m_device->CreateTexture2D(&depthDesc, nullptr, renderTexture->m_depthTexture.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("Graphics::CreateRenderTexture - DepthTexture 생성 실패", ErrorUtils::ToVerbose(hr));
			return nullptr;
		}

		hr = m_device->CreateDepthStencilView(renderTexture->m_depthTexture.Get(), nullptr, renderTexture->m_dsv.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("Graphics::CreateRenderTexture - DepthStencilView 생성 실패", ErrorUtils::ToVerbose(hr));
			return nullptr;
		}

		return renderTexture;
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
			ID3D11Buffer* rawBuffer = ib->GetRawBuffer();
			DXGI_FORMAT format = ConvertToDXGIFormat(ib->GetFormat());

			m_deviceContext->IASetIndexBuffer(
				rawBuffer,
				format,
				0
			);
		}
		else
		{
			m_deviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
		}

		m_currentIB = ib;
	}

	void Graphics::UpdateConstantBuffer(eCBufferSlot slot, const void* data, uint32_t size)
	{
		assert(m_deviceContext);

		uint32_t slotIndex = static_cast<uint32_t>(slot);

		if (m_constantBuffers[slotIndex] == nullptr || data == nullptr)
		{
			spdlog::error("Graphics::UpdateConstantBuffer - 비어있는 슬롯 [{}]", slotIndex);
			return;
		}

		D3D11_MAPPED_SUBRESOURCE mappedResource{};
		if (FAILED(m_deviceContext->Map(
			m_constantBuffers[slotIndex].Get(),
			0,
			D3D11_MAP_WRITE_DISCARD,
			0,
			&mappedResource
		)))
		{
			spdlog::error("Graphics::UpdateConstantBuffer - 슬롯 [{}] Map 실패", slotIndex);
			return;
		}

		std::memcpy(mappedResource.pData, data, size);
		m_deviceContext->Unmap(m_constantBuffers[slotIndex].Get(), 0);
	}

	void Graphics::BindConstantBuffer(eCBufferSlot slot)
	{
		assert(m_deviceContext);

		uint32_t slotIndex = static_cast<uint32_t>(slot);

		if (m_constantBuffers[slotIndex] == nullptr)
			return;

		ID3D11Buffer* buffer = m_constantBuffers[slotIndex].Get();

		m_deviceContext->VSSetConstantBuffers(slotIndex, 1, &buffer);
		m_deviceContext->PSSetConstantBuffers(slotIndex, 1, &buffer);
	}

	void Graphics::BindAllSamplers()
	{
		ID3D11SamplerState* samplers[static_cast<size_t>(eSamplerState::Count)] =
		{
			m_samplerStates[static_cast<size_t>(eSamplerState::WrapLinear)].Get(),
			m_samplerStates[static_cast<size_t>(eSamplerState::ClampPoint)].Get(),
			m_samplerStates[static_cast<size_t>(eSamplerState::ClampLinear)].Get(),
			m_samplerStates[static_cast<size_t>(eSamplerState::Skybox)].Get(),
			m_samplerStates[static_cast<size_t>(eSamplerState::ShadowCompare)].Get()
		};

		m_deviceContext->PSSetSamplers(0, static_cast<UINT>(eSamplerState::Count), samplers);
	}

	void Graphics::BindTexture(std::shared_ptr<Texture2D> tex)
	{
		if (tex == nullptr)
			return;

		auto srv = tex->GetShaderResourceView();
		m_deviceContext->PSSetShaderResources(0, 1, &srv);
	}

	void Graphics::BindTexture(UINT startSlot, std::shared_ptr<Texture2D> tex)
	{
		if (tex == nullptr)
			return;

		auto srv = tex->GetShaderResourceView();
		m_deviceContext->PSSetShaderResources(startSlot, 1, &srv);
	}

	bool Graphics::updateBackbuffer()
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
			spdlog::error("Graphics::updateBackbuffer - 백버퍼 획득 실패");
			return false;
		}

		hr = m_device->CreateRenderTargetView(static_cast<ID3D11Resource*>(backBuffer), nullptr, m_backbufferRTV.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("Graphics::updateBackbuffer - 백버퍼의 렌더타겟 뷰 생성 실패");
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
			spdlog::error("Graphics::updateBackbuffer - 백버퍼의 깊이 스텐실 버퍼 생성 실패");
			return false;
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc{};
		viewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipSlice = 0;

		hr = m_device->CreateDepthStencilView(static_cast<ID3D11Resource*>(m_backbufferTexture.Get()), &viewDesc, m_backbufferDSV.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("Graphics::updateBackbuffer - 백버퍼의 깊이 스텐실 뷰 생성 실패");
			return false;
		}

		m_width = static_cast<uint32_t>(desc.BufferDesc.Width);
		m_height = static_cast<uint32_t>(desc.BufferDesc.Height);

		return true;
	}

	bool Graphics::createDepthStencilStates()
	{
		assert(m_device);

		HRESULT hr = S_OK;

		D3D11_DEPTH_STENCIL_DESC desc{};

		// Depth Read Write
		desc.DepthEnable = TRUE;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_LESS;
		desc.StencilEnable = FALSE;
		desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		D3D11_DEPTH_STENCILOP_DESC stencilMarkOp = { D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS };
		desc.FrontFace = stencilMarkOp;
		desc.BackFace = stencilMarkOp;

		hr = m_device->CreateDepthStencilState(
			&desc,
			m_depthStencilStates[static_cast<size_t>(eDepthStencilState::DepthReadWrite)].GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("[::CreateDepthStencilStates] DepthReadWrite 생성 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		// DepthReadWrite_StencilReadWrite => Skydome에서 on
		// => rastertek에서 가장 최초에 사용하는 것
		ZeroMemory(&desc, sizeof(desc));
		desc.DepthEnable = TRUE;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_LESS;
		desc.StencilEnable = TRUE;
		desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		hr = m_device->CreateDepthStencilState(
			&desc,
			m_depthStencilStates[static_cast<size_t>(eDepthStencilState::DepthReadWrite_StencilReadWrite)].GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("[::CreateDepthStencilStates] DepthReadWrite_StencilReadWrite 생성 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		// GBuffer
		ZeroMemory(&desc, sizeof(desc));
		desc.DepthEnable = TRUE;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_LESS;
		desc.StencilEnable = TRUE;
		desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		stencilMarkOp = { D3D11_STENCIL_OP_REPLACE, D3D11_STENCIL_OP_REPLACE, D3D11_STENCIL_OP_REPLACE, D3D11_COMPARISON_ALWAYS };
		desc.FrontFace = stencilMarkOp;
		desc.BackFace = stencilMarkOp;

		hr = m_device->CreateDepthStencilState(
			&desc,
			m_depthStencilStates[static_cast<size_t>(eDepthStencilState::GBuffer)].GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("[::CreateDepthStencilStates] GBuffer 생성 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		// Depth Disabled
		ZeroMemory(&desc, sizeof(desc));
		desc.DepthEnable = FALSE;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_LESS;
		desc.StencilEnable = TRUE;
		desc.StencilReadMask = 0xFF;
		desc.StencilWriteMask = 0xFF;
		desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		hr = m_device->CreateDepthStencilState(
			&desc,
			m_depthStencilStates[static_cast<size_t>(eDepthStencilState::DepthDisabled)].GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("[::CreateDepthStencilStates] DepthDisabled 생성 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		// Forward cbLight
		desc.DepthEnable = TRUE;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		desc.StencilEnable = FALSE;
		desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		const D3D11_DEPTH_STENCILOP_DESC noSkyStencilOp = { D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_EQUAL };
		desc.FrontFace = noSkyStencilOp;
		desc.BackFace = noSkyStencilOp;

		hr = m_device->CreateDepthStencilState(
			&desc,
			m_depthStencilStates[static_cast<size_t>(eDepthStencilState::ForwardLight)].GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("[::CreateDepthStencilStates] ForwardLight 생성 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		// Transparent
		ZeroMemory(&desc, sizeof(desc));
		desc.DepthEnable = TRUE;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		desc.DepthFunc = D3D11_COMPARISON_LESS;

		hr = m_device->CreateDepthStencilState(
			&desc,
			m_depthStencilStates[static_cast<size_t>(eDepthStencilState::Transparent)].GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("[::CreateDepthStencilStates] Transparent 생성 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		// Skybox
		ZeroMemory(&desc, sizeof(desc));
		desc.DepthEnable = FALSE;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		desc.StencilEnable = FALSE;

		hr = m_device->CreateDepthStencilState(
			&desc,
			m_depthStencilStates[static_cast<size_t>(eDepthStencilState::Skybox)].GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("[::CreateDepthStencilStates] Skybox 생성 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		return true;
	}

	bool Graphics::createRasterizerStates()
	{
		assert(m_device);

		HRESULT hr = S_OK;

		D3D11_RASTERIZER_DESC desc{};

		// FillSolid_CullFront
		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_FRONT;
		desc.FrontCounterClockwise = TRUE;
		desc.DepthBias = 0;
		desc.DepthBiasClamp = 0.0f;
		desc.SlopeScaledDepthBias = 0.0f;
		desc.DepthClipEnable = TRUE;
		desc.ScissorEnable = FALSE;
		desc.MultisampleEnable = FALSE;
		desc.AntialiasedLineEnable = FALSE;

		hr = m_device->CreateRasterizerState(&desc, m_rasterizerStates[static_cast<size_t>(eRasterizerState::FillSolid_CullFront)].GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("[::CreateRasterizerStates] FillSolid_CullFront 생성 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		// FillSolid_CullBack
		desc.FrontCounterClockwise = FALSE;
		desc.CullMode = D3D11_CULL_BACK;

		hr = m_device->CreateRasterizerState(&desc, m_rasterizerStates[static_cast<size_t>(eRasterizerState::FillSolid_CullBack)].GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("[::CreateRasterizerStates] FillSolid_CullBack 생성 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		// FillSolid_CullNone
		desc.CullMode = D3D11_CULL_NONE;

		hr = m_device->CreateRasterizerState(&desc, m_rasterizerStates[static_cast<size_t>(eRasterizerState::FillSolid_CullNone)].GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("[::CreateRasterizerStates] FillSolid_CullNode 생성 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		return true;
	}

	bool Graphics::createBlendStates()
	{
		assert(m_device);

		HRESULT hr = S_OK;

		// Alpha
		{
			D3D11_BLEND_DESC desc{};
			desc.AlphaToCoverageEnable = FALSE;
			desc.IndependentBlendEnable = FALSE;

			const D3D11_RENDER_TARGET_BLEND_DESC alphaBlendDesc =
			{
				TRUE,                                // BlendEnable
				D3D11_BLEND_SRC_ALPHA,               // SrcBlend
				D3D11_BLEND_INV_SRC_ALPHA,           // DestBlend
				D3D11_BLEND_OP_ADD,                  // BlendOp

				D3D11_BLEND_ONE,                     // SrcBlendAlpha
				D3D11_BLEND_INV_SRC_ALPHA,           // DestBlendAlpha
				D3D11_BLEND_OP_ADD,                  // BlendOpAlpha

				D3D11_COLOR_WRITE_ENABLE_ALL         // RenderTargetWriteMask
			};

			for (UINT i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
				desc.RenderTarget[i] = alphaBlendDesc;

			hr = m_device->CreateBlendState(&desc, m_blendStates[static_cast<size_t>(eBlendState::AlphaEnabled)].GetAddressOf());
			if (FAILED(hr))
			{
				spdlog::error("[::CreateBlendState] AlphaEnabled 생성 실패: {}", ErrorUtils::ToVerbose(hr));
				return false;
			}

			for (UINT i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
				desc.RenderTarget[i].BlendEnable = FALSE;

			hr = m_device->CreateBlendState(&desc, m_blendStates[static_cast<size_t>(eBlendState::AlphaDisabled)].GetAddressOf());
			if (FAILED(hr))
			{
				spdlog::error("[::CreateBlendState] AlpahDisabled 생성 실패: {}", ErrorUtils::ToVerbose(hr));
				return false;
			}
		}

		// Additive
		{
			D3D11_BLEND_DESC desc{};
			desc.AlphaToCoverageEnable = FALSE;
			desc.IndependentBlendEnable = FALSE;
			const D3D11_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
			{
				TRUE,
				D3D11_BLEND_ONE,
				D3D11_BLEND_ONE,
				D3D11_BLEND_OP_ADD,

				D3D11_BLEND_ONE,
				D3D11_BLEND_ONE,
				D3D11_BLEND_OP_ADD,

				D3D11_COLOR_WRITE_ENABLE_ALL,
			};
			for (UINT i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
				desc.RenderTarget[i] = defaultRenderTargetBlendDesc;

			hr = m_device->CreateBlendState(&desc, m_blendStates[static_cast<size_t>(eBlendState::Additive)].GetAddressOf());
			if (FAILED(hr))
			{
				spdlog::error("[::CreateBlendState] Additive 생성 실패: {}", ErrorUtils::ToVerbose(hr));
				return false;
			}
		}

		return true;
	}

	bool Graphics::createSamplerStates()
	{
		assert(m_device);

		HRESULT hr = S_OK;

		// WrapLinear
		{
			D3D11_SAMPLER_DESC samplerDesc{};
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.MipLODBias = 0.0f;
			samplerDesc.MaxAnisotropy = 1;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			samplerDesc.BorderColor[0] = 0;
			samplerDesc.BorderColor[1] = 0;
			samplerDesc.BorderColor[2] = 0;
			samplerDesc.BorderColor[3] = 0;
			samplerDesc.MinLOD = 0;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

			hr = m_device->CreateSamplerState(&samplerDesc, m_samplerStates[static_cast<size_t>(eSamplerState::WrapLinear)].GetAddressOf());
			if (FAILED(hr))
			{
				spdlog::error("[::CreateSamplerStates] WrapLinear Sampler 생성 실패: {}", ErrorUtils::ToVerbose(hr));
				return false;
			}
		}

		// ClampPoint
		{
			D3D11_SAMPLER_DESC samplerDesc{};
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

			hr = m_device->CreateSamplerState(&samplerDesc, m_samplerStates[static_cast<size_t>(eSamplerState::ClampPoint)].GetAddressOf());
			if (FAILED(hr))
			{
				spdlog::error("[::CreateSamplerStates] ClampPoint Sampler 생성 실패: {}", ErrorUtils::ToVerbose(hr));
				return false;
			}
		}

		// ClampLinear
		{
			D3D11_SAMPLER_DESC samplerDesc{};
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;


			hr = m_device->CreateSamplerState(&samplerDesc, m_samplerStates[static_cast<size_t>(eSamplerState::ClampLinear)].GetAddressOf());
			if (FAILED(hr))
			{
				spdlog::error("[::CreateSamplerStates] ClampLinear Sampler 생성 실패: {}", ErrorUtils::ToVerbose(hr));
				return false;
			}
		}

		// Skybox
		{
			D3D11_SAMPLER_DESC samplerDesc{};
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			samplerDesc.MinLOD = 0;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

			hr = m_device->CreateSamplerState(&samplerDesc, m_samplerStates[static_cast<size_t>(eSamplerState::Skybox)].GetAddressOf());
			if (FAILED(hr))
			{
				spdlog::error("[::CreateSamplerStates] Skybox Sampler 생성 실패: {}", ErrorUtils::ToVerbose(hr));
				return false;
			}
		}

		// ShadowCompare
		{
			D3D11_SAMPLER_DESC samplerDesc{};
			samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
			samplerDesc.BorderColor[0] = 1.0f;
			samplerDesc.BorderColor[1] = 1.0f;
			samplerDesc.BorderColor[2] = 1.0f;
			samplerDesc.BorderColor[3] = 1.0f;
			samplerDesc.MinLOD = 0.0f;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
			samplerDesc.MipLODBias = 0.0f;

			hr = m_device->CreateSamplerState(&samplerDesc, m_samplerStates[static_cast<size_t>(eSamplerState::ShadowCompare)].GetAddressOf());
			if (FAILED(hr))
			{
				spdlog::error("[::CreateSamplerStates] ShadowCompare Sampler 생성 실패: {}", ErrorUtils::ToVerbose(hr));
				return false;
			}
		}

		return true;
	}

	bool Graphics::createConstantBuffers()
	{
		assert(m_device);

		D3D11_BUFFER_DESC desc{};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.ByteWidth = 1024;	// 넉넉한 크기로 생성

		for (uint32_t i = 0; i < static_cast<uint32_t>(eCBufferSlot::Count); ++i)
		{
			if (FAILED(m_device->CreateBuffer(&desc, nullptr, m_constantBuffers[i].GetAddressOf())))
			{
				spdlog::error("Graphics::createConstantBuffers - 슬롯 [{}]생성 실패", i);
				return false;
			}
		}

		return true;
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
