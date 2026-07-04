#pragma once
#include "Texture.h"

#include <wrl/client.h>

namespace Dive
{
	// 윈도우 크기 변화에 어떻게 대응할 것인지 좀 더 생각해보자.
	// 이 곳에서 직접 Graphics를 호출하고 싶지는 않다.
	class RenderTexture : public Texture
	{
		friend class Graphics;

	public:
		RenderTexture() = default;
		RenderTexture(uint32_t width, uint32_t height, 
			DXGI_FORMAT colorFormat = DXGI_FORMAT_R16G16B16A16_FLOAT,
			DXGI_FORMAT depthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT);
		virtual ~RenderTexture() override;

		DXGI_FORMAT GetColorFormat() const { return m_colorFormat; }
		DXGI_FORMAT GetDepthFormat() const { return m_depthFormat; }

		ID3D11RenderTargetView* GetRenderTargetView() const { return m_rtv.Get(); }
		ID3D11DepthStencilView* GetDepthStencilView() const { return m_dsv.Get(); }

	private:
		DXGI_FORMAT m_colorFormat = DXGI_FORMAT_UNKNOWN;
		DXGI_FORMAT m_depthFormat = DXGI_FORMAT_UNKNOWN;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsv;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthTexture;
	};
}