#include "stdafx.h"
#include "Cubemap.h"
//#include "Core/CoreSystem.h"
#include "Graphics/GraphicsSystem.h"

namespace Dive
{
	DvCubemap::DvCubemap(uint32_t size, DXGI_FORMAT format, bool useMips)
		:  m_faceData{}
	{
		m_info.width = size;
		m_info.height = size;
		m_info.format = format;
		m_info.useMips = useMips;
		m_info.mipLevels = useMips && CanGenerateMips(format) ? CalculateMipmapLevels(size, size) : 1;

		SetName("Cubemap");
	}

	DvCubemap::~DvCubemap()
	{
		Release();
	}

	bool DvCubemap::Serialize(std::ofstream& fout) const
	{
		if (!fout.good())
			return false;

		fout.write(reinterpret_cast<const char*>(&m_info), sizeof(DvTexture::BaseInfo));

		for (int i = 0; i < 6; ++i)
		{
			uint32_t faceSize = static_cast<uint32_t>(m_faceData[i].size());
			fout.write(reinterpret_cast<const char*>(&faceSize), sizeof(faceSize));

			if (faceSize > 0)
			{
				fout.write(reinterpret_cast<const char*>(m_faceData[i].data()), faceSize);
			}
		}

		return true;
	}

	bool DvCubemap::Deserialize(std::ifstream& fin)
	{
		if (!fin.good())
			return false;

		fin.read(reinterpret_cast<char*>(&m_info), sizeof(DvTexture::BaseInfo));

		for (int i = 0; i < 6; ++i)
		{
			uint32_t faceSize = 0;
			fin.read(reinterpret_cast<char*>(&faceSize), sizeof(uint32_t));

			if (faceSize > 0)
			{
				m_faceData[i].resize(faceSize);
				fin.read(reinterpret_cast<char*>(m_faceData[i].data()), faceSize);
			}
		}

		return true;
	}

	bool DvCubemap::Create()
	{
		// 1. Texture2D 설명 설정 (큐브맵용)
		D3D11_TEXTURE2D_DESC texDesc{};
		texDesc.Format = m_info.format;
		texDesc.Width = static_cast<UINT>(m_info.width);
		texDesc.Height = static_cast<UINT>(m_info.height);
		texDesc.MipLevels = static_cast<UINT>(m_info.mipLevels);
		texDesc.ArraySize = 6; // 큐브맵은 항상 6
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		// MiscFlags 설정 (TEXTURECUBE 필수)
		texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		if (m_info.useMips && m_info.mipLevels == 1)
			texDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;

		// 2. 리소스 생성
		auto hr = GraphicsDevice::GetDevice()->CreateTexture2D(&texDesc, nullptr, m_texture.GetAddressOf());
		if (FAILED(hr))
		{
			DV_LOG(DvCubemap, eLogLevel::Err, "[::Create] CreateTexture2D 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		// 3. 데이터 전송 (각 Face별로 Update)
		if (!m_faceData[0].empty())
		{
			if (std::any_of(m_faceData.begin(), m_faceData.end(), [](const auto& data) { return data.empty(); }))
			{
				DV_LOG(DvCubemap, eLogLevel::Err, "[::Create] 일부 큐브맵 데이터가 비어있음");
				return false;
			}

			// 익스포터에서 전달받은 정확한 rowPitch 사용
			UINT rowPitch = static_cast<UINT>(m_info.rowPitch);

			for (int i = 0; i < 6; ++i)
			{
				// D3D11CalcSubresource(MipSlice, ArraySlice, TotalMipLevels)
				// 각 면(i)의 0번 밉레벨에 데이터 복사
				UINT subresourceIndex = D3D11CalcSubresource(0, static_cast<UINT>(i), m_info.mipLevels);

				GraphicsDevice::GetDeviceContext()->UpdateSubresource(
					m_texture.Get(),
					subresourceIndex,
					nullptr,
					m_faceData[i].data(),
					rowPitch, // 한 줄의 바이트 크기
					0         // 2D 텍스처/큐브맵 면에서는 Depth Pitch가 0 (3D 텍스처에서만 사용)
				);
			}
		}

		// 4. Render Target View 생성 (필요 시)
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = m_info.format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.FirstArraySlice = 0;
		rtvDesc.Texture2DArray.ArraySize = 6;

		hr = GraphicsDevice::GetDevice()->CreateRenderTargetView(m_texture.Get(), &rtvDesc, m_renderTargetView.GetAddressOf());
		if (FAILED(hr))
		{
			DV_LOG(DvCubemap, eLogLevel::Err, "[::Create] CreateRenderTargetView 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		// 5. Shader Resource View 생성 (TEXTURECUBE 차원 확인)
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = m_info.format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = m_info.mipLevels;
		srvDesc.TextureCube.MostDetailedMip = 0;

		hr = GraphicsDevice::GetDevice()->CreateShaderResourceView(m_texture.Get(), &srvDesc, m_shaderResourceView.GetAddressOf());
		if (FAILED(hr))
		{
			DV_LOG(DvCubemap, eLogLevel::Err, "[::Create] CreateShaderResourceView 실패 : {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		// 6. 밉맵 생성 및 메모리 정리
		if (m_info.useMips && (texDesc.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS))
			GraphicsDevice::GetDeviceContext()->GenerateMips(m_shaderResourceView.Get());

		for (auto& face : m_faceData)
		{
			face.clear();
			face.shrink_to_fit();
		}

		return true;
	}

	void DvCubemap::Release()
	{
		for (auto& data : m_faceData)
			data.clear();

		m_renderTargetView.Reset();
	}
	/*
	bool DvCubemap::LoadFromFaceFiles(const std::vector<std::filesystem::path>& filepaths, bool useMips)
	{
		if (filepaths.size() != 6)
		{
			DV_LOG(DvCubemap, eLogLevel::Err, "[::LoadFromFaceFiles] 모자란 개수 전달: {}", filepaths.size());
			return false;
		}

		uint32_t size = 0;
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

		std::array<std::vector<uint8_t>, 6> faceData;

		for (uint32_t i = 0; i < 6; ++i)
		{
			const auto& filepath = filepaths[i];
			if (!std::filesystem::exists(filepath))
			{
				DV_LOG(DvCubemap, eLogLevel::Err, "[::LoadFromFaceFiles] 존재하지 않는 파일 전달: {}", filepath.string());
				return false;
			}

			// 텍스처 로드
			DirectX::ScratchImage image;
			if (FAILED(DirectX::LoadFromWICFile(filepath.c_str(), DirectX::WIC_FLAGS_IGNORE_SRGB, nullptr, image)))
			{
				DV_LOG(DvCubemap, eLogLevel::Err, "[::LoadFromFaceFiles] WIC 파일 로드 실패: {}", filepath.string());
				return false;
			}

			// 첫 번째 Face에서 크기와 포맷 결정
			if (i == 0)
			{
				size = static_cast<uint32_t>(image.GetMetadata().width);
				format = image.GetMetadata().format;
			}

			// 유효성 검사
			if (image.GetMetadata().width != size || image.GetMetadata().height != size)
			{
				DV_LOG(DvCubemap, eLogLevel::Err, "[::LoadFromFaceFiles] 잘못된 크기 설정: {} != {}", image.GetMetadata().width, size);
				return false;
			}

			// Face 데이터 저장
			const DirectX::Image* img = image.GetImage(0, 0, 0);
			if (!img)
			{
				DV_LOG(DvCubemap, eLogLevel::Err, "[::LoadFromFaceFiles] 유효하지 않은 이미지 데이터: {}", filepath.string());
				return false;
			}

			faceData[i].assign(img->pixels, img->pixels + (img->rowPitch * img->height));
		}

		m_size = size;
		m_format = format;
		m_useMips = useMips;

		for (uint32_t i = 0; i < 6; ++i)
			SetFaceData(i, faceData[i].data(), faceData[i].size());

		return true;
	}
	*/
	void DvCubemap::SetFaceData(uint32_t index, const void* pixels, size_t size)
	{
		if (index > 5)
		{
			DV_LOG(DvCubemap, eLogLevel::Warn, "[::SetFaceData] 잘못된 Face 인덱스 전달: {}", index);
			return;
		}

		m_faceData[index].resize(size);
		std::copy(static_cast<const uint8_t*>(pixels), static_cast<const uint8_t*>(pixels) + size, m_faceData[index].begin());
	}

	// ========================================================================================================================================

	Cubemap::Cubemap(uint32_t size, DXGI_FORMAT format, bool useMips)
		: m_size(size), m_faceData{}
	{
		m_format = format;
		m_useMips = useMips;

		SetName("Cubemap");
	}

	Cubemap::~Cubemap()
	{
		Release();
	}

	bool Cubemap::SaveToFile(const std::filesystem::path& filepath)
	{
		DV_LOG(Cubemap, eLogLevel::Warn, "[::SaveToFile] 미구현");
		return false;
	}

	bool Cubemap::LoadFromFile(const std::filesystem::path& filepath)
	{
		auto target = filepath.string() + ".dds";
		std::ifstream fin(target, std::ios::binary);
		if (!fin.is_open())
		{
			DV_LOG(Cubemap, eLogLevel::Err, "[::LoadFromFile] 파일 열기 실패: {}", target);
			return false;
		}

		if (!Deserialize(fin))
			return false;

		ClearDirty();
		return true;
	}

	bool Cubemap::Serialize(std::ofstream& fout) const
	{
		DV_LOG(Cubemap, eLogLevel::Warn, "[::Serialize] 미구현");
		return false;
	}

	bool Cubemap::Deserialize(std::ifstream& fin)
	{
		if (!fin.good())
		{
			DV_LOG(Cubemap, eLogLevel::Err, "[::Deserialize]");
			return false;
		}

		std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
		
		// DDS 파일 로드
		DirectX::ScratchImage image;
		if (FAILED(DirectX::LoadFromDDSMemory((const std::byte*)buffer.data(), buffer.size(), DirectX::DDS_FLAGS_NONE, nullptr, image)))
		{
			DV_LOG(Cubemap, eLogLevel::Err, "[::Deserialize]");
			return false;
		}

		// 메타데이터 확인
		const auto& metadata = image.GetMetadata();
		if (!(metadata.miscFlags & DirectX::TEX_MISC_TEXTURECUBE))
		{
			DV_LOG(Cubemap, eLogLevel::Err, "[::Deserialize]");
			return false;
		}

		m_size = static_cast<uint32_t>(metadata.width);
		m_format = metadata.format;
		m_useMips = false;

		// 각 Face 데이터 설정
		for (uint32_t i = 0; i < 6; ++i)
		{
			const DirectX::Image* img = image.GetImage(0, i, 0); // 6개의 Face 중 하나를 가져옴
			if (!img)
			{
				DV_LOG(Cubemap, eLogLevel::Err, "[::Deserialize]");
				return false;
			}

			SetFaceData(i, img->pixels, img->rowPitch * img->height);
		}

		return true;
	}

	bool Cubemap::Create()
	{
		m_mipLevels = CanGenerateMips(m_format) ? (m_useMips ? CalculateMipmapLevels(m_size, m_size) : 1) : 1;

		D3D11_TEXTURE2D_DESC texDesc{};
		texDesc.Format = m_format;
		texDesc.Width = static_cast<UINT>(m_size);
		texDesc.Height = static_cast<UINT>(m_size);
		texDesc.MipLevels = static_cast<UINT>(m_mipLevels);
		texDesc.ArraySize = 6;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | (m_useMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0);

		auto hr = GraphicsDevice::GetDevice()->CreateTexture2D(&texDesc, nullptr, m_texture.GetAddressOf());
		if (FAILED(hr))
		{
			DV_LOG(Cubemap, eLogLevel::Err, "[::Create] CreateTexture2D 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		if (!m_faceData[0].empty())
		{
			if (std::any_of(m_faceData.begin(), m_faceData.end(), [](const auto& data) { return data.empty(); }))
			{
				DV_LOG(Cubemap, eLogLevel::Err, "[::Create] 빈 규브맵 데이터");
				return false;
			}

			UINT rowPitch = static_cast<UINT>(m_size * GetPixelSize(m_format));
			for (int i = 0; i < 6; ++i)
			{
				GraphicsDevice::GetDeviceContext()->UpdateSubresource(
					m_texture.Get(),
					D3D11CalcSubresource(0, static_cast<UINT>(i), m_mipLevels),
					nullptr,
					(const void*)m_faceData[i].data(),
					rowPitch,
					rowPitch * m_size);	// 조금 의심스럽다. 나중에 확인
			}
		}

		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = m_format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.FirstArraySlice = 0;
		rtvDesc.Texture2DArray.ArraySize = 6;

		hr = GraphicsDevice::GetDevice()->CreateRenderTargetView(static_cast<ID3D11Resource*>(m_texture.Get()), &rtvDesc, m_renderTargetView.GetAddressOf());
		if (FAILED(hr))
		{
			DV_LOG(Cubemap, eLogLevel::Err, "[::Create] CreateRenderTargetView 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = m_format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = m_mipLevels;

		hr = GraphicsDevice::GetDevice()->CreateShaderResourceView(static_cast<ID3D11Resource*>(m_texture.Get()), &srvDesc, m_shaderResourceView.GetAddressOf());
		if (FAILED(hr))
		{
			DV_LOG(Cubemap, eLogLevel::Err, "[::Create] CreateShaderResourceView 실패 : {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		if (m_useMips && m_shaderResourceView)	GraphicsDevice::GetDeviceContext()->GenerateMips(m_shaderResourceView.Get());

		return true;
	}

	void Cubemap::Release()
	{
		for (auto& data : m_faceData)
			data.clear();

		m_renderTargetView.Reset();
	}

	bool Cubemap::LoadFromFaceFiles(const std::vector<std::filesystem::path>& filepaths, bool useMips)
	{
		if (filepaths.size() != 6)
		{
			DV_LOG(Cubemap, eLogLevel::Err, "[::LoadFromFaceFiles] 모자란 개수 전달: {}", filepaths.size());
			return false;
		}

		uint32_t size = 0;
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

		std::array<std::vector<uint8_t>, 6> faceData;

		for (uint32_t i = 0; i < 6; ++i)
		{
			const auto& filepath = filepaths[i];
			if (!std::filesystem::exists(filepath))
			{
				DV_LOG(Cubemap, eLogLevel::Err, "[::LoadFromFaceFiles] 존재하지 않는 파일 전달: {}", filepath.string());
				return false;
			}

			// 텍스처 로드
			DirectX::ScratchImage image;
			if (FAILED(DirectX::LoadFromWICFile(filepath.c_str(), DirectX::WIC_FLAGS_IGNORE_SRGB, nullptr, image)))
			{
				DV_LOG(Cubemap, eLogLevel::Err, "[::LoadFromFaceFiles] WIC 파일 로드 실패: {}", filepath.string());
				return false;
			}

			// 첫 번째 Face에서 크기와 포맷 결정
			if (i == 0)
			{
				size = static_cast<uint32_t>(image.GetMetadata().width);
				format = image.GetMetadata().format;
			}

			// 유효성 검사
			if (image.GetMetadata().width != size || image.GetMetadata().height != size)
			{
				DV_LOG(Cubemap, eLogLevel::Err, "[::LoadFromFaceFiles] 잘못된 크기 설정: {} != {}", image.GetMetadata().width, size);
				return false;
			}

			// Face 데이터 저장
			const DirectX::Image* img = image.GetImage(0, 0, 0);
			if (!img)
			{
				DV_LOG(Cubemap, eLogLevel::Err, "[::LoadFromFaceFiles] 유효하지 않은 이미지 데이터: {}", filepath.string());
				return false;
			}

			faceData[i].assign(img->pixels, img->pixels + (img->rowPitch * img->height));
		}

		m_size = size;
		m_format = format;
		m_useMips = useMips;

		for (uint32_t i = 0; i < 6; ++i)
			SetFaceData(i, faceData[i].data(), faceData[i].size());

		return true;
	}

	void Cubemap::SetFaceData(uint32_t index, const void* pixels, size_t size)
	{
		if (index > 5) 
		{
			DV_LOG(Cubemap, eLogLevel::Warn, "[::SetFaceData] 잘못된 Face 인덱스 전달: {}", index);
			return;
		}

		m_faceData[index].resize(size);
		std::copy(static_cast<const uint8_t*>(pixels), static_cast<const uint8_t*>(pixels) + size, m_faceData[index].begin());
	}
}
