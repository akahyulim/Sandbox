#include "pch.h"
#include "Texture2D.h"
#include "Graphics/Graphics.h"

#include <DirectXTex/DirectXTex.h>

namespace Dive
{
	DvTexture2D::DvTexture2D()
	{
		SetName("DvTexture2D");
	}

	DvTexture2D::DvTexture2D(uint32_t width, uint32_t height, DXGI_FORMAT format, bool useMips)
	{
		m_info.width = width;
		m_info.height = height;
		m_info.format = format;
		m_info.useMips = useMips;
		m_info.mipLevels = useMips && CanGenerateMips(format) ? CalculateMipmapLevels(width, height) : 1;

		SetName("DvTexture2D");
	}

	DvTexture2D::~DvTexture2D()
	{
		Release();
		spdlog::info("소멸: {}", GetName());
	}

	bool DvTexture2D::Serialize(std::ofstream& fout) const
	{
		if (!fout.good())
		{
			spdlog::error("[::Serialize]");
			return false;
		}

		fout.write(reinterpret_cast<const char*>(&m_info), sizeof(DvTexture::BaseInfo));

		uint32_t dataSize = static_cast<uint32_t>(m_pixelData.size());
		fout.write(reinterpret_cast<const char*>(&dataSize), sizeof(uint32_t));

		if (dataSize > 0)
		{
			fout.write(reinterpret_cast<const char*>(m_pixelData.data()), dataSize);
		}

		return true;
	}

	bool DvTexture2D::Deserialize(std::ifstream& fin)
	{
		if (!fin.good())
		{
			spdlog::error("[::Deserialize]");
			return false;
		}

		fin.read(reinterpret_cast<char*>(&m_info), sizeof(DvTexture::BaseInfo));

		uint32_t dataSize = 0;
		fin.read(reinterpret_cast<char*>(&dataSize), sizeof(uint32_t));

		if (dataSize > 0)
		{
			m_pixelData.resize(dataSize);
			fin.read(reinterpret_cast<char*>(m_pixelData.data()), dataSize);
		}

		return true;
	}

	void DvTexture2D::SetPixelData(const void* pixels, size_t size)
	{
		m_pixelData.resize(size);
		std::copy(static_cast<const uint8_t*>(pixels), static_cast<const uint8_t*>(pixels) + size, m_pixelData.begin());
	}
	
	bool DvTexture2D::Create(Graphics* graphics)
	{
		if (m_pixelData.empty())
		{
			spdlog::error("[::Create] 빈 픽셀 데이터로 시도");
			return false;
		}

		// 1. Texture2D 리소스 설명 설정
		D3D11_TEXTURE2D_DESC texDesc{};
		texDesc.Width = static_cast<UINT>(m_info.width);
		texDesc.Height = static_cast<UINT>(m_info.height);
		texDesc.MipLevels = static_cast<UINT>(m_info.mipLevels); // 에셋 툴에서 구운 밉 레벨 개수 (예: 10)
		texDesc.ArraySize = 1;
		texDesc.Format = m_info.format;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;

		// 이제 런타임에 동적으로 만들 필요가 없으므로 가장 최적화된 설정을 사용합니다.
		// 데이터가 변경되지 않으므로 IMMUTABLE이 최상이지만, CreateTexture2D 호출 시 
		// pInitialData를 넘겨야 하므로 여기서는 일단 DEFAULT나 IMMUTABLE 중 선택합니다.
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texDesc.MiscFlags = 0;

		// 2. 텍스처 리소스 생성 (데이터 없이 뼈대만 먼저 생성)
		//HRESULT hr = GraphicsDevice::GetDevice()->CreateTexture2D(&texDesc, nullptr, m_texture.GetAddressOf());
		
		if (FAILED(hr))
		{
			spdlog::error("[::Create] CreateTexture2D 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		// 3. 모든 밉맵 레벨 데이터 전송
		uint8_t* pCurrentMipData = m_pixelData.data();

		for (UINT i = 0; i < texDesc.MipLevels; ++i)
		{
			// 각 밉 레벨의 해상도 계산
			UINT currWidth = std::max<UINT>(1, texDesc.Width >> i);
			UINT currHeight = std::max<UINT>(1, texDesc.Height >> i);

			// 각 밉 레벨에 맞는 RowPitch와 SlicePitch 계산
			// (CalculateRowPitch/CalculateSlicePitch는 에셋 툴과 동일한 로직이어야 함)
			UINT currRowPitch = CalculateRowPitch(m_info.format, currWidth);
			UINT currSlicePitch = CalculateSlicePitch(m_info.format, currWidth, currHeight);

			// 해당 밉 레벨(i)에 데이터 복사
			GraphicsDevice::GetDeviceContext()->UpdateSubresource(
				m_texture.Get(),
				i,              // 밉 레벨 인덱스
				nullptr,
				pCurrentMipData,
				currRowPitch,
				0
			);

			// 다음 밉 레벨 데이터 위치로 포인터 이동
			pCurrentMipData += currSlicePitch;
		}

		// 4. Shader Resource View 생성
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;

		hr = GraphicsDevice::GetDevice()->CreateShaderResourceView(
			m_texture.Get(),
			&srvDesc,
			m_shaderResourceView.GetAddressOf()
		);

		if (FAILED(hr))
		{
			spdlog::error("[::Create] CreateShaderResourceView 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		// 5. GPU 연산(GenerateMips)은 이제 필요 없으므로 삭제되었습니다.

		// 6. CPU 메모리 즉시 해제
		m_pixelData.clear();
		m_pixelData.shrink_to_fit();

		return true;
	}

	void DvTexture2D::Release()
	{
		m_pixelData.clear();
	}

	// ====================================================================================================================================================

	Texture2D::Texture2D()
	{
		SetName("Texture2D");
	}

	Texture2D::Texture2D(uint32_t width, uint32_t height, DXGI_FORMAT format, bool useMips)
	{
		m_width = width;
		m_height = height;
		m_format = format;
		m_useMips = useMips;

		SetName("Texture2D");
	}

	Texture2D::~Texture2D()
	{
		Release();
		spdlog::info("소멸: {}", GetName());
	}

	bool Texture2D::SaveToFile(const std::filesystem::path& filepath)
	{
		spdlog::warn("[::SaveToFile] 미구현");
		return false;
	}

	bool Texture2D::LoadFromFile(const std::filesystem::path& filepath)
	{
		static const std::vector<std::string> exts = { ".png", ".jpg", ".jpeg" , ".bmp", ".tga" };
		std::string target{};
		for (const auto& ext : exts)
		{
			auto candidate = filepath.string() + ext;
			if (std::filesystem::exists(candidate))
			{
				target = candidate;
				m_extension = ext;
				break;
			}
		}

		std::ifstream fin(target, std::ios::binary);
		if (!fin.is_open())
		{
			spdlog::error("[::LoadFromFile] 파일 열기 실패: {}", target);
			return false;
		}

		if (!Deserialize(fin))
			return false;

		ClearDirty();
		return true;
	}

	bool Texture2D::Serialize(std::ofstream& fout) const
	{
		if (!fout.good())
		{
			spdlog::error("[::Serialize]");
			return false;
		}

		spdlog::warn("[::Serialize] 미구현");
		return true;
	}

	bool Texture2D::Deserialize(std::ifstream& fin)
	{
		if (!fin.good())
		{
			spdlog::error("[::Deserialize]");
			return false;
		}

		std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(fin)),std::istreambuf_iterator<char>());
		DirectX::ScratchImage img{};
		HRESULT hr = 0;

		if (m_extension == ".dds")
			hr = DirectX::LoadFromDDSMemory((const std::byte*)buffer.data(), buffer.size(), DirectX::DDS_FLAGS_NONE, nullptr, img);
		else if (m_extension == ".tga")
			hr = DirectX::LoadFromTGAMemory((const uint8_t*)buffer.data(), buffer.size(), nullptr, img);
		else
			hr = DirectX::LoadFromWICMemory((const std::byte*)buffer.data(), buffer.size(), DirectX::WIC_FLAGS_NONE, nullptr, img);

		if (FAILED(hr))
		{
			spdlog::error("[::Deserialize] 텍스쳐 메모리 로드 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		m_width = static_cast<uint32_t>(img.GetImages()->width);
		m_height = static_cast<uint32_t>(img.GetImages()->height);
		m_format = img.GetImages()->format;

		SetPixelData((const void*)img.GetImages()->pixels, img.GetImages()->slicePitch);
		
		return true;
	}

	void Texture2D::SetPixelData(const void* pixels, size_t size)
	{
		m_pixelData.resize(size);
		std::copy(static_cast<const uint8_t*>(pixels), static_cast<const uint8_t*>(pixels) + size, m_pixelData.begin());
	}

	bool Texture2D::Create(Graphics* graphics)
	{
		if (m_pixelData.empty())
		{
			spdlog::error("[::Create] 빈 픽셀 데이터로 시도");
			return false;
		}

		m_mipLevels = CanGenerateMips(m_format) ? (m_useMips ? CalculateMipmapLevels(m_width, m_height) : 1) : 1;

		// texture2d
		D3D11_TEXTURE2D_DESC texDesc{};
		texDesc.Format = m_format;
		texDesc.Width = static_cast<UINT>(m_width);
		texDesc.Height = static_cast<UINT>(m_height);
		texDesc.ArraySize = 1;
		texDesc.MipLevels = static_cast<UINT>(m_mipLevels);
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		texDesc.MiscFlags = m_useMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
		texDesc.CPUAccessFlags = 0;

		auto hr = GraphicsDevice::GetDevice()->CreateTexture2D(&texDesc, nullptr, m_texture.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("[::Create] CreateTexture2D 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		uint32_t rowPitch = m_width * GetPixelSize(m_format);
		GraphicsDevice::GetDeviceContext()->UpdateSubresource(
			static_cast<ID3D11Resource*>(m_texture.Get()),
			0, nullptr,
			m_pixelData.data(),
			rowPitch,
			0);

		// ShaderResourceView
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = m_format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = static_cast<UINT>(m_mipLevels);
		srvDesc.Texture2D.MostDetailedMip = 0;

		hr = GraphicsDevice::GetDevice()->CreateShaderResourceView(static_cast<ID3D11Resource*>(m_texture.Get()), &srvDesc, m_shaderResourceView.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("[::Create] CreateShaderReosurceView 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		if (m_useMips && m_shaderResourceView)	GraphicsDevice::GetDeviceContext()->GenerateMips(m_shaderResourceView.Get());

		m_pixelData.clear();
		m_pixelData.shrink_to_fit();

		return true;
	}

	void Texture2D::Release()
	{
		m_pixelData.clear();
	}

	bool Texture2D::LoadFromMemory(const std::filesystem::path& filepath, const void* sourceData, size_t size, bool useMips)
	{
		auto extension = filepath.extension().string();

		DirectX::ScratchImage img;
		HRESULT hr = 0;
		if (extension == "dds")
			hr = DirectX::LoadFromDDSMemory((const std::byte*)sourceData, size, DirectX::DDS_FLAGS_NONE, nullptr, img);
		else if (extension == "tga")
			hr = DirectX::LoadFromTGAMemory((const uint8_t*)sourceData, size, nullptr, img);
		else
			hr = DirectX::LoadFromWICMemory((const std::byte*)sourceData, size, DirectX::WIC_FLAGS_NONE, nullptr, img);

		if (FAILED(hr))
		{
			spdlog::error("[::LoadFromMemory] 텍스쳐 메모리 로드 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		m_width = static_cast<uint32_t>(img.GetImages()->width);
		m_height = static_cast<uint32_t>(img.GetImages()->height);
		m_format = img.GetImages()->format;
		m_useMips = useMips;

		SetPixelData((const void*)img.GetImages()->pixels, img.GetImages()->rowPitch * img.GetImages()->height);
		
		return true;
	}
}
