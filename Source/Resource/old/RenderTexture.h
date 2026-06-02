#pragma once
#include <d3d11.h>
#include <dxgi.h>
#include <cstdint>
#include <wrl/client.h>
#include <filesystem>
#include <fstream>
#include "Texture.h"

namespace Dive
{
	enum class eDepthFormat
	{
		None = 0,
		Depth16 = 16,
		Depth24Stencil8 = 32
	};

	class DvRenderTexture : public DvTexture
	{
	public:
		DvRenderTexture() = delete;
		DvRenderTexture(uint32_t width, uint32_t height, eDepthFormat depth = eDepthFormat::None, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, bool useMips = false);
		~DvRenderTexture() override;

		virtual bool Create() override;
		virtual void Release() override;

		bool Resize(uint32_t width, uint32_t height);

		ID3D11RenderTargetView* GetRenderTargetView() const { return m_renderTargetView.Get(); }
		ID3D11DepthStencilView* GetDepthStencilView() const { return m_depthDSV.Get(); }
		ID3D11ShaderResourceView* GetDepthStencilShaderResourceView() const { return m_depthSRV.Get(); }

		eResourceType GetType() const override { return eResourceType::RenderTexture; }
		static constexpr eResourceType StaticType() { return eResourceType::RenderTexture; }

	protected:
		virtual bool Serialize(std::ofstream& fout) const override;
		virtual bool Deserialize(std::ifstream& fin) override;

	private:
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthTexture;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthDSV;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_depthSRV;

		eDepthFormat m_depthFormat = eDepthFormat::None;
	};

	class RenderTexture : public Texture
	{
	public:
		RenderTexture() = delete;
		RenderTexture(uint32_t width, uint32_t height, eDepthFormat depth = eDepthFormat::None, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, bool useMips = false);
		~RenderTexture() override;

		virtual bool SaveToFile(const std::filesystem::path& filepath) override;
		virtual bool LoadFromFile(const std::filesystem::path& filepath) override;

		virtual bool Create() override;
		virtual void Release() override;
		
		bool Resize(uint32_t width, uint32_t height);

		ID3D11RenderTargetView* GetRenderTargetView() const { return m_renderTargetView.Get(); }
		ID3D11DepthStencilView* GetDepthStencilView() const { return m_depthDSV.Get(); }
		ID3D11ShaderResourceView* GetDepthStencilShaderResourceView() const { return m_depthSRV.Get(); }

		eResourceType GetType() const override { return eResourceType::RenderTexture; }
		static constexpr eResourceType StaticType() { return eResourceType::RenderTexture; }

	protected:
		virtual bool Serialize(std::ofstream& fout) const override;
		virtual bool Deserialize(std::ifstream& fin) override;

	private:
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthTexture;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthDSV;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_depthSRV;

		eDepthFormat m_depthFormat = eDepthFormat::None;
	};
}
