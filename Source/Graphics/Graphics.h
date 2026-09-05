#pragma once
#include <wrl/client.h>
#include <span>
#include <d3d11_3.h>
#include <DXGI1_3.h>

#include "Core/Types.h"
#include "Shader.h"
#include "RenderPass.h"

namespace Dive
{
	class Window;
	class VertexBuffer;
	class IndexBuffer;
	class InputLayout;
	class DepthStencilState;
	class RasterizerState;
	class BlendState;
	class SamplerState;

	class Graphics
	{
	public:
		explicit Graphics(Window* window);
		Graphics(Graphics const&) = delete;
		Graphics(Graphics&&) = default;
		Graphics& operator=(Graphics const&) = delete;
		Graphics& operator=(Graphics&&) = default;
		~Graphics();

		void SetBackbuffer();
		void ClearBackbuffer();
		void ResizeBackbuffer(uint32_t width, uint32_t height);

		void SwapBuffers(bool vSync);

		// 의외로 이 두 메서드가 adrian엔 없다...
		uint32_t GetWidth() const { return m_width; }
		uint32_t GetHeight() const { return m_height; }

		Window* GetWindow() const { return m_window; }

		IDXGISwapChain* GetSwapChain() const { return m_swapChain.Get(); }
		ID3D11Device* GetDevice() const { return m_device.Get(); }
		ID3D11DeviceContext* GetDeviceContext() const { return m_deviceContext.Get(); }

		ID3D11RenderTargetView* GetRenderTargetView() const { return m_backbufferRTV.Get(); }

		// device
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& desc);
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> CreateRasterizerState(const D3D11_RASTERIZER_DESC& desc);
		Microsoft::WRL::ComPtr<ID3D11BlendState> CreateBlendState(const D3D11_BLEND_DESC& desc);
		Microsoft::WRL::ComPtr<ID3D11SamplerState> CreateSamplerState(const D3D11_SAMPLER_DESC& desc);

		// context
		void UpdateBuffer(ID3D11Buffer* cbuffer, const void* data, uint32_t size);

		void SetVertexBuffer(VertexBuffer* vb, uint32_t slot = 0);
		void SetVertexBuffers(uint32_t startSlot, std::span<VertexBuffer*> vbs);
		void SetIndexBuffer(IndexBuffer* ib);
		void SetConstantBuffer(eShaderStage stage, uint32_t slot, ID3D11Buffer* const* ppBuffer);

		void SetDepthStencilState(ID3D11DepthStencilState* dss, uint32_t stencilRef);
		void SetRasterizerState(ID3D11RasterizerState* rs);
		void SetBlendState(ID3D11BlendState* bs, float* blendFactor = nullptr, uint32_t smaplerMask = 0xFFFFFFFF);

		void SetVertexShader(VertexShader* vs);
		void SetPixelShader(PixelShader* ps);
		void SetComputeShader(ComputeShader* cs);
		void SetInputLayout(InputLayout* il);

		void SetShaderResourceView(eShaderStage stage, uint32_t slot, ID3D11ShaderResourceView* const* ppSrv);

		void SetTopology(ePrimitiveTopology topology);
		void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
		void SetViewport(const Viewport& vp);
		void SetViewport(uint32_t width, uint32_t height);

		void BeginRenderPass(const RenderPassDesc& desc);
		void EndRenderPass();

		void Draw(uint32_t vertexCount, uint32_t startVertexLocation = 0);
		void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation = 0, int32_t baseVertexLocation = 0);


	private:
		void createBackbufferResources(uint32_t width, uint32_t height);

	private:
		Window* m_window = nullptr;
		uint32_t m_width;
		uint32_t m_height;

		Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
		Microsoft::WRL::ComPtr<ID3D11Device> m_device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_backbufferRTV;

		VertexShader* m_currentVS = nullptr;
		PixelShader* m_currentPS = nullptr;
		ComputeShader* m_currentCS = nullptr;
		InputLayout* m_currentIL = nullptr;

		ID3D11DepthStencilState* m_currentDSS = nullptr;
		ID3D11RasterizerState* m_currentRS = nullptr;
		ID3D11BlendState* m_currentBS = nullptr;

		ePrimitiveTopology m_currentTopology = ePrimitiveTopology::None;
	};
}