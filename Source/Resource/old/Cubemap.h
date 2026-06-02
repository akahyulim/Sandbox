#pragma once
#include <dxgi.h>
#include <d3d11.h>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>
#include <array>
#include <wrl/client.h>
#include "Texture.h"

namespace Dive
{
	class DvCubemap : public DvTexture
	{
	public:
		DvCubemap() = default;
		DvCubemap(uint32_t size, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, bool useMips = true);
		~DvCubemap() override;

		virtual bool Create() override;
		virtual void Release() override;

		//bool LoadFromFaceFiles(const std::vector<std::filesystem::path>& filepaths, bool useMips = true);

		uint32_t GetSize() const { return m_info.width; }

		void SetFaceData(uint32_t index, const void* pixels, size_t size);

		ID3D11RenderTargetView* GetRenderTargetView() const { return m_renderTargetView.Get(); }

		eResourceType GetType() const override { return eResourceType::Cubemap; }
		static constexpr eResourceType StaticType() { return eResourceType::Cubemap; }

	protected:
		virtual bool Serialize(std::ofstream& fout) const override;
		virtual bool Deserialize(std::ifstream& fin) override;

	private:
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
		std::array<std::vector<uint8_t>, 6> m_faceData;
	};

	class Cubemap : public Texture
	{
	public:
		Cubemap() = default;
		Cubemap(uint32_t size, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, bool useMips = true);
		~Cubemap() override;

		virtual bool SaveToFile(const std::filesystem::path& filepath) override;
		virtual bool LoadFromFile(const std::filesystem::path& filepath) override;

		virtual bool Create() override;
		virtual void Release() override;

		bool LoadFromFaceFiles(const std::vector<std::filesystem::path>& filepaths, bool useMips = true);

		uint32_t GetSize() const { return m_size; }

		void SetFaceData(uint32_t index, const void* pixels, size_t size);

		ID3D11RenderTargetView* GetRenderTargetView() const { return m_renderTargetView.Get(); }

		eResourceType GetType() const override { return eResourceType::Cubemap; }
		static constexpr eResourceType StaticType() { return eResourceType::Cubemap; }

	protected:
		virtual bool Serialize(std::ofstream& fout) const override;
		virtual bool Deserialize(std::ifstream& fin) override;

	private:
		uint32_t m_size = 0;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
		std::array<std::vector<uint8_t>, 6> m_faceData;
	};
}
