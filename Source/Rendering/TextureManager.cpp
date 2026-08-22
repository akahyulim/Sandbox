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
	
	void TextureManager::Destory()
	{
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

	/*
	TextureHandle TextureManager::LoadCubemap(const std::wstring& filepath)
	{
		auto it = m_loaded.find(filepath);
		if (it != m_loaded.end())
			return it->second;

		{
			TextureFormat format = GetTextureFormat(filepath);
			assert(format == TextureFormat::DDS && "지원하지 않는 포맷입니다. DDS 파일만 지원합니다.");
		}

		std::ifstream fin(filepath, std::ios::binary);
		if (!fin.is_open())
		{
			spdlog::error("[::LoadFromFile] 파일 열기 실패: {}", StringUtils::ToString(filepath));
			return INVALID_TEXTURE_HANDLE;
		}

		std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());

		// DDS 파일 로드
		DirectX::ScratchImage image;
		if (FAILED(DirectX::LoadFromDDSMemory((const std::byte*)buffer.data(), buffer.size(), DirectX::DDS_FLAGS_NONE, nullptr, image)))
		{
			spdlog::error("[::Deserialize]");
			return INVALID_TEXTURE_HANDLE;
		}

		// 메타데이터 확인
		const auto& metadata = image.GetMetadata();
		if (!(metadata.miscFlags & DirectX::TEX_MISC_TEXTURECUBE))
		{
			spdlog::error("[::Deserialize]");
			return INVALID_TEXTURE_HANDLE;
		}

		auto size = static_cast<uint32_t>(metadata.width);
		DXGI_FORMAT format = metadata.format;
		auto useMips = false;

		// 각 Face 데이터 설정
		for (uint32_t i = 0; i < 6; ++i)
		{
			const DirectX::Image* img = image.GetImage(0, i, 0); // 6개의 Face 중 하나를 가져옴
			if (!img)
			{
				spdlog::error("[Deserialize]");
				return INVALID_TEXTURE_HANDLE;
			}

			SetFaceData(i, img->pixels, img->rowPitch * img->height);
		}

		auto mipLevels = 1;

		ID3D11Device* device = m_graphics->GetDevice();
		ID3D11DeviceContext* context = m_graphics->GetDeviceContext();
		
		D3D11_TEXTURE2D_DESC texDesc{};
		texDesc.Format = format;
		texDesc.Width = static_cast<UINT>(size);
		texDesc.Height = static_cast<UINT>(size);
		texDesc.MipLevels = static_cast<UINT>(mipLevels);
		texDesc.ArraySize = 6;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		auto hr = device->CreateTexture2D(&texDesc, nullptr, texture.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("[::Create] CreateTexture2D 실패: {}", ErrorUtils::ToVerbose(hr));
			return INVALID_TEXTURE_HANDLE;
		}

		if (!m_faceData[0].empty())
		{
			if (std::any_of(m_faceData.begin(), m_faceData.end(), [](const auto& data) { return data.empty(); }))
			{
				spdlog::error("[::Create] 빈 규브맵 데이터");
				return INVALID_TEXTURE_HANDLE;
			}

			UINT rowPitch = static_cast<UINT>(size * GetPixelSize(format));
			for (int i = 0; i < 6; ++i)
			{
				context->UpdateSubresource(
					texture.Get(),
					D3D11CalcSubresource(0, static_cast<UINT>(i), mipLevels),
					nullptr,
					(const void*)m_faceData[i].data(),
					rowPitch,
					rowPitch * size);	// 조금 의심스럽다. 나중에 확인
			}
		}

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = mipLevels;

		hr = device->CreateShaderResourceView(static_cast<ID3D11Resource*>(texture.Get()), &srvDesc, srv.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("[::Create] CreateShaderResourceView 실패 : {}", ErrorUtils::ToVerbose(hr));
			return INVALID_TEXTURE_HANDLE;
		}

		// 4. 매니저 캐시에 등록 후 핸들 반환
		++m_handle;
		TextureHandle handle = m_handle;
		m_loaded.insert({ filepath, handle });
		m_textures.insert({ handle, std::move(srv) });

		return handle;
	}
	*/
	
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
		m_textures.insert({ handle, std::move(srv) });

		return handle;
	}

	ID3D11ShaderResourceView* TextureManager::GetTextureView(TextureHandle handle) const
	{
		auto it = m_textures.find(handle);
		if (it != m_textures.end())
			return it->second.Get();

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
		m_loaded.insert({ filepath, m_handle });
		m_textures.insert({ m_handle, std::move(srv) });
		return m_handle;
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