#pragma once
#include <wrl/client.h>
#include <vector>
#include <d3d11.h>

namespace Dive
{
	class Graphics;

	class RenderTexture
	{
	public:
		RenderTexture(Graphics* graphics, const D3D11_TEXTURE2D_DESC& desc, D3D11_SUBRESOURCE_DATA* data = nullptr);
		~RenderTexture() = default;

		ID3D11Texture2D* GetResource() const { return m_texture2D.Get(); }
		ID3D11ShaderResourceView* GetShaderResourceView() const { return m_shaderResourceView.Get(); }
		ID3D11RenderTargetView* GetRenderTargetView(uint32_t index = 0) const;
		ID3D11DepthStencilView* GetDetphStencilView(uint32_t index = 0) const;
		//ID3D11UnorderedAccessView* GetUnorderedAccessView() const { return m_unorderedAccessView.Get(); }

		uint32_t GetWidth() const { return m_desc.Width; }
		uint32_t GetHeight() const { return m_desc.Height; }

	private:
		void createShaderResourceView();
		void createRenderTargetViews();
		void createDepthStencilViews();

	private:
		Graphics* m_graphics = nullptr;
		D3D11_TEXTURE2D_DESC m_desc{};

		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture2D;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
		std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> m_renderTargetViews;
		std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> m_depthStencilViews;
		//Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_unorderedAccessView;
	};
}