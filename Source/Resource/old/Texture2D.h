#pragma once
#include <cstdint>
#include <dxgi.h>
#include <fstream>
#include <filesystem>

#include "Texture.h"

namespace Dive
{
	class Graphics;

	class DvTexture2D : public DvTexture
	{
	public:
		DvTexture2D();
		DvTexture2D(uint32_t width, uint32_t height, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, bool useMips = false);
		~DvTexture2D() override;

		virtual bool Create(Graphics* graphics) override;
		virtual void Release() override;

		void SetPixelData(const void* pixels, size_t size);

		eResourceType GetType() const override { return eResourceType::Texture2D; }
		static constexpr eResourceType StaticType() { return eResourceType::Texture2D; }

	protected:
		bool Serialize(std::ofstream& fout) const override;
		bool Deserialize(std::ifstream& fin) override;

	private:
		std::vector<uint8_t> m_pixelData;
	};

	class Texture2D : public Texture
	{
	public:
		Texture2D();
		Texture2D(uint32_t width, uint32_t height, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, bool useMips = false);
		~Texture2D() override;

		virtual bool SaveToFile(const std::filesystem::path& filepath) override;
		virtual bool LoadFromFile(const std::filesystem::path& filepath) override;

		virtual bool Create(Graphics* graphics) override;
		virtual void Release() override;

		void SetPixelData(const void* pixels, size_t size);

		bool LoadFromMemory(const std::filesystem::path& filepath, const void* sourceData, size_t size, bool useMips = false);

		eResourceType GetType() const override { return eResourceType::Texture2D; }
		static constexpr eResourceType StaticType() { return eResourceType::Texture2D; }

	protected:
		bool Serialize(std::ofstream& fout) const override;
		bool Deserialize(std::ifstream& fin) override;

	private:
		std::vector<uint8_t> m_pixelData;
		std::string m_extension{};
	};
}
