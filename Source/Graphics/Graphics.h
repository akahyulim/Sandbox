#pragma once
#include <array>
#include <cstdint>
#include <wrl/client.h>
#include <d3d11_3.h>
#include <DXGI1_3.h>
#include <Windows.h>
#include <spdlog/spdlog.h>

#include "Core/Types.h"
#include "PipelineState.h"
#include "RenderPass.h"

namespace Dive
{
	class VertexBuffer;
	class IndexBuffer;
	class ConstantBuffer;
	class VertexShader;
	class PixelShader;
	class InputLayout;

	class Graphics
	{
	public:
		Graphics() = default;
		~Graphics();

		bool Initialize(HWND hWnd, uint32_t width, uint32_t height, bool windowed);

		void Resize(uint32_t  width, uint32_t height);
		void OnResizeViews();

		void SetPipelineState(const PipelineState& state);
		void BeginRenderPass(const RenderPass& pass);
		void EndRenderPass();
		void BindMainRenderTarget();

		void Draw(uint32_t vertexCount, uint32_t startVertexLocation = 0);
		void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation = 0, int32_t baseVertexLocation = 0);

		void Present() const;

		template<typename T>
		std::unique_ptr<ConstantBuffer> CreateConstantBuffer();
		std::unique_ptr<VertexBuffer> CreateVertexBuffer(uint32_t stride, uint32_t count, const void* data);
		std::unique_ptr<IndexBuffer> CreateIndexBuffer(eFormat format, uint32_t count, const void* data);

		std::shared_ptr<VertexShader> CreateVertexShader(const void* byteCode, size_t size);
		std::shared_ptr<PixelShader> CreatePixelShader(const void* byteCode, size_t size);
		std::shared_ptr<InputLayout> CreateInputLayout(eInputLayout type, const void* byteCode, size_t size);

		void BindVertexBuffer(VertexBuffer* vb);
		void BindIndexBuffer(IndexBuffer* ib);
		void BindVSConstantBuffer(eCBufferSlotVS slot, ConstantBuffer* cb);
		void BindPSConstantBuffer(eCBufferSlotPS slot, ConstantBuffer* cb);

		// 이것도 바꿔야 한다. BackBuffer랑 Cur의 구분이 필요하다.
		ID3D11RenderTargetView* GetRenderTargetView() const { return m_backbufferRTV.Get(); }
		ID3D11DepthStencilView* GetDepthStencilView() const { return m_backbufferDSV.Get(); }

		IDXGISwapChain* GetSwapChain() const { return m_swapChain.Get(); }
		ID3D11Device* GetDevice() const { return m_device.Get(); }
		ID3D11DeviceContext* GetDeviceContext() const { return m_deviceContext.Get(); }

		uint32_t GetWidth() const { return m_width; }
		uint32_t GetHeight() const { return m_height; }
		
		bool VSyncEnabled() const { return m_vSync; }
		void SetVSyncEnabled(bool enabled) { m_vSync = enabled; }

	private:
		bool setupViews();
		bool createDepthStencilStates();
		bool createRasterizerStates();
		bool createBlendStates();
		bool createSamplerStates();
		bool resizeSwapChain();

		void bindVertexShader(ID3D11VertexShader* vs);
		void bindPixelShader(ID3D11PixelShader* ps);
		void bindInputLayout(ID3D11InputLayout* il);
		void bindPrimitiveTopology(ePrimitiveTopology topology);
		void bindDepthStencilState(ID3D11DepthStencilState* dss, uint32_t stencilRef);
		void bindRasterizerState(ID3D11RasterizerState* rs);
		void bindBlendState(ID3D11BlendState* bs, const float* blendFactor, uint32_t sampleMask);
		void bindSamplerState(uint32_t slot, ID3D11SamplerState* ss);

	private:
		Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
		Microsoft::WRL::ComPtr<ID3D11Device> m_device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_backbufferRTV;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_backbufferTexture;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_backbufferDSV;

		uint32_t m_width;
		uint32_t m_height;

		bool m_vSync = false;

		std::array<std::unique_ptr<ID3D11DepthStencilState>, static_cast<size_t>(eDepthStencilState::Count)> m_depthStencilStates;
		std::array<std::unique_ptr<ID3D11RasterizerState>, static_cast<size_t>(eRasterizerState::Count)> m_rasterizerStates;
		std::array<std::unique_ptr<ID3D11BlendState>, static_cast<size_t>(eBlendState::Count)> m_blendStates;
		std::array<std::unique_ptr<ID3D11SamplerState>, static_cast<size_t>(eSamplerState::Count)> m_samplerStates;

		uint32_t m_currentStencilRef = 0;
		float m_currentBlendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		uint32_t m_currentSampleMask = 0xFFFFFFFF;
		std::array<ID3D11SamplerState*, 16> m_currentSamplers = { nullptr };

		ID3D11DepthStencilState* m_currentDSS = nullptr;
		ID3D11RasterizerState* m_currentRS = nullptr;
		ID3D11BlendState* m_currentBS = nullptr;
		ID3D11SamplerState* m_currentSS = nullptr;

		uint32_t m_currentRTVCount = 0;
		ID3D11RenderTargetView* m_currentRTVs[8] = { nullptr };
		ID3D11DepthStencilView* m_currentDSV = nullptr;
		
		VertexBuffer* m_currentVB = nullptr;
		IndexBuffer* m_currentIB = nullptr;
		ID3D11Buffer* m_currentVSCB[14] = { nullptr };
		ID3D11Buffer* m_currentPSCB[14] = { nullptr };

		ID3D11VertexShader* m_currentVS = nullptr;
		ID3D11PixelShader* m_currentPS = nullptr;
		ID3D11InputLayout* m_currentLayout = nullptr;

		ePrimitiveTopology m_currentTopology = ePrimitiveTopology::None;
		Viewport m_currentViewport;
	};

	template<typename T>
	std::unique_ptr<ConstantBuffer> Graphics::CreateConstantBuffer()
	{
		auto cb = std::make_unique<ConstantBuffer>(sizeof(T));

		D3D11_BUFFER_DESC desc{};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = static_cast<UINT>(cb->GetSize());
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		if (FAILED(m_device->CreateBuffer(&desc, nullptr, cb->GetAddressOf())))
		{
			spdlog::error("상수 버퍼 생성 실패");
			return nullptr;
		}

		return cb;
	}
}