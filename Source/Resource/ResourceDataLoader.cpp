#include "pch.h"
#include "ResourceDataLoader.h"

namespace Dive
{
	std::unique_ptr<ResourceDataLoader::TextureResult> ResourceDataLoader::LoadTextureData(const std::filesystem::path& filepath)
	{
		// 1. 🔍 파일 유효성 검사
		if (!std::filesystem::exists(filepath) || !std::filesystem::is_regular_file(filepath))
		{
			spdlog::error("ResourceDataLoader::LoadTextureData - 파일이 없거나 올바르지 않습니다: {}", filepath.string());
			return nullptr;
		}

		std::wstring extension = filepath.extension().wstring();

		auto metaData = std::make_unique<DirectX::TexMetadata>();
		auto scratchImage = std::make_unique<DirectX::ScratchImage>();
		HRESULT hr = S_OK;

		// 💡 대소문자 무시 확장자 비교 람다 (유저님의 완벽한 코드 유지!)
		auto EqualsIgnoreCase = [](std::wstring_view a, std::wstring_view b) {
			return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](wchar_t charA, wchar_t charB) {
				return std::towlower(charA) == std::towlower(charB);
				});
			};

		// 2. 📁 확장자별 파일 로더 호출 (디스크에서 직접 읽는 API 사용)
		if (EqualsIgnoreCase(extension, L".dds"))
		{
			hr = DirectX::LoadFromDDSFile(
				filepath.c_str(),
				DirectX::DDS_FLAGS_NONE,
				metaData.get(),
				*scratchImage
			);
		}
		else if (EqualsIgnoreCase(extension, L".tga"))
		{
			hr = DirectX::LoadFromTGAFile(
				filepath.c_str(),
				DirectX::TGA_FLAGS_NONE,
				metaData.get(),
				*scratchImage
			);
		}
		else // .png, .jpg, .jpeg, .bmp 등 WIC(Windows Imaging Component) 지원 포맷
		{
			hr = DirectX::LoadFromWICFile(
				filepath.c_str(),
				DirectX::WIC_FLAGS_NONE,
				metaData.get(),
				*scratchImage
			);
		}

		// 3. 🚨 로딩 에러 체크
		if (FAILED(hr))
		{
			spdlog::error("ResourceDataLoader::LoadTextureData - 이미지 파싱 실패 (확장자: {}): {}, 에러코드: {}",
				filepath.extension().string(), filepath.string(), ErrorUtils::ToVerbose(hr));
			return nullptr;
		}

		// 4. 🎁 완제품 재료 패키징 후 반환
		auto result = std::make_unique<TextureResult>();
		result->scratchImage = std::move(scratchImage);
		result->metaData = std::move(metaData);

		return result;
	}
}