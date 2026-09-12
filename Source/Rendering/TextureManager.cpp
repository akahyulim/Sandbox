#include "pch.h"
#include "TextureManager.h"
#include "Core/Common.h"
#include "Graphics/Graphics.h"
#include "Utilities/FileUtils.h"
#include "Utilities/StringUtils.h"

namespace Dive
{
	namespace
	{
		enum class TextureFormat
		{
			DDS,
			BMP,
			JPG,
			PNG,
			TIFF,
			GIF,
			ICO,
			TGA,
			HDR,
			PIC,
			NotSupported
		};

		TextureFormat GetTextureFormat(std::string const& path)
		{
			std::string extension = FileUtils::GetExtension(path);
			std::transform(extension.begin(), extension.end(), extension.begin(),
				[](unsigned char c) { return std::tolower(c); });

			if (extension == ".dds")
				return TextureFormat::DDS;
			else if (extension == ".bmp")
				return TextureFormat::BMP;
			else if (extension == ".jpg" || extension == ".jpeg")
				return TextureFormat::JPG;
			else if (extension == ".png")
				return TextureFormat::PNG;
			else if (extension == ".tiff" || extension == ".tif")
				return TextureFormat::TIFF;
			else if (extension == ".gif")
				return TextureFormat::GIF;
			else if (extension == ".ico")
				return TextureFormat::ICO;
			else if (extension == ".tga")
				return TextureFormat::TGA;
			else if (extension == ".hdr")
				return TextureFormat::HDR;
			else if (extension == ".pic")
				return TextureFormat::PIC;
			else
				return TextureFormat::NotSupported;
		}
		TextureFormat GetTextureFormat(std::wstring const& path)
		{
			return GetTextureFormat(StringUtils::ToString(path));
		}
		constexpr uint32_t MipmapLevels(uint32_t width, uint32_t height)
		{
			uint32_t levels = 1U;
			while ((width | height) >> levels) ++levels;
			return levels;
		}

		uint32_t GetPixelSize(DXGI_FORMAT format)
		{
			switch (format)
			{
			case DXGI_FORMAT_R8G8B8A8_UNORM:
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			case DXGI_FORMAT_B8G8R8A8_UNORM:
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			case DXGI_FORMAT_R32_FLOAT:
				return 4;
			case DXGI_FORMAT_R16G16B16A16_FLOAT:
				return 8;
			case DXGI_FORMAT_R32G32B32A32_FLOAT:
				return 16;
			default:
				//DV_LOG(Texture, warn, "[::GetPixelSize] 잘못된 포멧 전달");
				return 0;
			}
		}
	}

	void TextureManager::Initialize(Graphics* graphics)
	{
		m_graphics = graphics;
	}
	
	void TextureManager::Shutdown()
	{
		m_textures.clear();
		m_loaded.clear();

		m_graphics = nullptr;
	}
	
	TextureHandle TextureManager::LoadTexture(const std::wstring& filepath, bool mips)
	{
		auto it = m_loaded.find(filepath);
		if (it != m_loaded.end())
			return it->second;

		TextureFormat format = GetTextureFormat(filepath);

		switch (format)
		{
		case TextureFormat::DDS:
			return loadDDSTexture(filepath, mips);
		case TextureFormat::TGA:
			return loadTGATexture(filepath, mips);
		case TextureFormat::BMP:
		case TextureFormat::PNG:
		case TextureFormat::JPG:
		case TextureFormat::TIFF:
		case TextureFormat::GIF:
		case TextureFormat::ICO:
			return loadWICTexture(filepath, mips);
		case TextureFormat::NotSupported:
		default:
			spdlog::error("지원하지 않는 텍스쳐 포멧");
		}

		return INVALID_TEXTURE_HANDLE;
	}
	
	TextureHandle TextureManager::LoadTexture(const std::string& name, bool mips)
	{
		return LoadTexture(StringUtils::ToWString(name), mips);
	}
	
	TextureHandle TextureManager::LoadCubemap(const std::wstring& filepath)
	{
		auto it = m_loaded.find(filepath);
		if (it != m_loaded.end())
			return it->second;

		TextureFormat format = GetTextureFormat(filepath);
		assert(format == TextureFormat::DDS && "지원하지 않는 포맷입니다. DDS 파일만 지원합니다.");

		DirectX::TexMetadata metadata{};
		DirectX::ScratchImage scratchImage;

		HRESULT hr = DirectX::LoadFromDDSFile(filepath.c_str(), DirectX::DDS_FLAGS_NONE, &metadata, scratchImage);
		if (FAILED(hr))
		{
			spdlog::error("DDS 파일 로드 실패: {}", ErrorUtils::ToVerbose(hr));
			return INVALID_TEXTURE_HANDLE;
		}

		if (!metadata.IsCubemap())
		{
			spdlog::error("큐브맵 텍스쳐가 아닙니다: {}", StringUtils::ToString(filepath));
			return INVALID_TEXTURE_HANDLE;
		}

		ID3D11Device* device = m_graphics->GetDevice();

		Microsoft::WRL::ComPtr<ID3D11Resource> texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;

		hr = DirectX::CreateTexture(
			device,
			scratchImage.GetImages(),
			scratchImage.GetImageCount(),
			metadata,
			texture.GetAddressOf()
		);
		if (FAILED(hr))
		{
			spdlog::error("큐브맵 텍스쳐 생성 실패: {}", ErrorUtils::ToVerbose(hr));
			return INVALID_TEXTURE_HANDLE;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = metadata.format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.MipLevels = static_cast<UINT>(metadata.mipLevels);

		hr = device->CreateShaderResourceView(texture.Get(), &srvDesc, srv.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("큐브맵 SRV 생성 실패: {}", ErrorUtils::ToVerbose(hr));
			return INVALID_TEXTURE_HANDLE;
		}

		++m_handle;
		TextureHandle handle = m_handle;
		m_loaded.insert({ filepath, handle });

		TextureInfo info{};
		info.filepath = filepath;
		info.name = std::filesystem::path(filepath).stem().string();
		info.srv = std::move(srv);
		m_textures.emplace(handle, std::move(info));

		return handle;
	}
	
	std::wstring TextureManager::GetTextureFilepath(TextureHandle handle) const
	{
		auto it = m_textures.find(handle);
		if (it != m_textures.end())
			return it->second.filepath;

		return std::wstring();
	}
	
	std::string TextureManager::GetTextureName(TextureHandle handle) const
	{
		auto it = m_textures.find(handle);
		if (it != m_textures.end())
			return it->second.name;

		return std::string();
	}

	ID3D11ShaderResourceView* TextureManager::GetTextureView(TextureHandle handle) const
	{
		auto it = m_textures.find(handle);
		if (it != m_textures.end())
			return it->second.srv.Get();

		return nullptr;
	}

	TextureHandle TextureManager::loadDDSTexture(const std::wstring& filepath, bool mips)
	{
		DirectX::TexMetadata metaData{};
		DirectX::ScratchImage scratchImage;

		auto hr = DirectX::LoadFromDDSFile(
			filepath.c_str(),
			DirectX::DDS_FLAGS_NONE,
			&metaData,
			scratchImage
		);

		if (FAILED(hr))
		{
			spdlog::error("Load DDS File 실패: {}", ErrorUtils::ToVerbose(hr));
			return INVALID_TEXTURE_HANDLE;
		}

		return createTexture(filepath, scratchImage, metaData, mips);
	}

	TextureHandle TextureManager::loadTGATexture(const std::wstring& filepath, bool mips)
	{
		DirectX::TexMetadata metaData{};
		DirectX::ScratchImage scratchImage;

		auto hr = DirectX::LoadFromTGAFile(
			filepath.c_str(),
			DirectX::TGA_FLAGS_NONE,
			&metaData,
			scratchImage
		);
		if (FAILED(hr))
		{
			spdlog::error("Load TAG Texture 실패: {}", ErrorUtils::ToVerbose(hr));
			return INVALID_TEXTURE_HANDLE;
		}

		return createTexture(filepath, scratchImage, metaData, mips);
	}

	TextureHandle TextureManager::loadWICTexture(const std::wstring& filepath, bool mips)
	{
		DirectX::TexMetadata metaData{};
		DirectX::ScratchImage scratchImage;

		auto hr = DirectX::LoadFromWICFile(
			filepath.c_str(),
			DirectX::WIC_FLAGS_NONE,
			&metaData,
			scratchImage
		);
		if (FAILED(hr))
		{
			spdlog::error("Load WIC Texture 실패: {}", ErrorUtils::ToVerbose(hr));
			return INVALID_TEXTURE_HANDLE;
		}

		return createTexture(filepath, scratchImage, metaData, mips);
	}

	TextureHandle TextureManager::createTexture(const std::wstring& filepath, DirectX::ScratchImage& scratchImage, DirectX::TexMetadata& metaData, bool mips)
	{
		{
			if (metaData.IsCubemap())
			{
				spdlog::info("cubemap: {}", StringUtils::ToString(filepath));
			}
		}

		if (mips && metaData.mipLevels == 1)
		{
			DirectX::ScratchImage mipChain;
			auto hr = DirectX::GenerateMipMaps(
				scratchImage.GetImages(),
				scratchImage.GetImageCount(),
				metaData,
				DirectX::TEX_FILTER_DEFAULT,
				0,
				mipChain
			);

			if (FAILED(hr))
			{
				spdlog::error("밉맵 생성 실패: {}", ErrorUtils::ToVerbose(hr));
				return INVALID_TEXTURE_HANDLE;
			}
			else
			{
				scratchImage = std::move(mipChain);
			}
		}

		ID3D11Device* device = m_graphics->GetDevice();
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
	
		auto hr = DirectX::CreateShaderResourceView(
			device,
			scratchImage.GetImages(),
			scratchImage.GetImageCount(),
			metaData,
			srv.GetAddressOf());

		if (FAILED(hr))
		{
			spdlog::error("Create ShaderResourceView 실패: {}", ErrorUtils::ToVerbose(hr));
			return INVALID_TEXTURE_HANDLE;
		}

		++m_handle;
		TextureHandle handle = m_handle;
		m_loaded.insert({ filepath, handle });

		TextureInfo info{};
		info.filepath = filepath;
		info.name = std::filesystem::path(filepath).stem().string();
		info.srv = std::move(srv);
		m_textures.emplace(handle, std::move(info));

		return handle;
	}

	void TextureManager::SetFaceData(uint32_t index, const void* pixels, size_t size)
	{
		if (index > 5)
		{
			//DV_LOG(Cubemap, warn, "[::SetFaceData] 잘못된 Face 인덱스 전달: {}", index);
			return;
		}

		m_faceData[index].resize(size);
		std::copy(static_cast<const uint8_t*>(pixels), static_cast<const uint8_t*>(pixels) + size, m_faceData[index].begin());
	}
}