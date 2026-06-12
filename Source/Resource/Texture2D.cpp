#include "pch.h"
#include "Texture2D.h"
#include "Graphics/Graphics.h"

namespace Dive
{
    Texture2D::~Texture2D() = default;

    bool Texture2D::Deserialize(const std::vector<uint8_t>& fileBuffer, std::wstring_view extension)
    {
        m_metaData = std::make_unique<DirectX::TexMetadata>();
        m_scratchImage = std::make_unique<DirectX::ScratchImage>();

        HRESULT hr = S_OK;

        // 💡 std::wstring_view 환경에서 대소문자 무시하고 확장자 비교하기
        // 부모가 넘겨준 확장자가 ".dds" 나 ".DDS" 인지 뷰 단위에서 바로 비교합니다.
        auto EqualsIgnoreCase = [](std::wstring_view a, std::wstring_view b) {
            return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](wchar_t charA, wchar_t charB) {
                return std::towlower(charA) == std::towlower(charB);
                });
            };

        if (EqualsIgnoreCase(extension, L".dds"))
        {
            hr = DirectX::LoadFromDDSMemory(fileBuffer.data(), fileBuffer.size(), DirectX::DDS_FLAGS_NONE, m_metaData.get(), *m_scratchImage);
        }
        else if (EqualsIgnoreCase(extension, L".tga"))
        {
            hr = DirectX::LoadFromTGAMemory(
                fileBuffer.data(), fileBuffer.size(),
                DirectX::TGA_FLAGS_NONE, // 필요시 TGA_FLAGS_NONE 사용
                m_metaData.get(), *m_scratchImage
            );
        }
        else // .png, .jpg, .bmp 등
        {
            hr = DirectX::LoadFromWICMemory(fileBuffer.data(), fileBuffer.size(), DirectX::WIC_FLAGS_NONE, m_metaData.get(), *m_scratchImage);
        }

        if (FAILED(hr)) return false;

        m_width = static_cast<uint32_t>(m_metaData->width);
        m_height = static_cast<uint32_t>(m_metaData->height);
        return true;
    }

    bool Texture2D::Create(Graphics* graphics)
    {
        if (graphics == nullptr)
        {
            spdlog::error("Texture2D::Create - Graphics 장치 포인터가 nullptr입니다.");
            return false;
        }

        // Deserialize가 먼저 성공적으로 수행되었는지 안전장치 검사
        if (!m_scratchImage || !m_metaData)
        {
            spdlog::error("Texture2D::Create - 디코딩된 이미지 메모리(ScratchImage)가 존재하지 않습니다.");
            return false;
        }

        // 💡 Graphics 공장장에게 내 알맹이들을 넘겨서 실물 SRV를 구워오라고 명령합니다!
        // 성공하면 m_srv.GetAddressOf()를 통해 실물 자원 주소가 내 방에 쏙 들어옵니다.
        if(!graphics->CreateTexture2D(
            m_scratchImage.get(),
            m_metaData.get(),
            m_srv.GetAddressOf()
        ))
        {
            spdlog::error("Texture2D::Create - Graphics 장치에서 SRV 생성에 실패했습니다.");
            return false;
        }

        // 💡 [메모리 최적화] GPU 자원이 생성되었으므로, 더 이상 RAM에 무거운 픽셀 데이터를 
        // 쥐고 있을 필요가 없습니다. 깔끔하게 릴리즈해서 메모리를 아낍니다!
        m_scratchImage.reset();
        m_metaData.reset();

        return true;
    }
}