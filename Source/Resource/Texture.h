#pragma once
#include "Resource.h"

#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <vector>
#include <memory>

namespace DirectX
{
	class ScratchImage;
	struct TexMetadata;
}

namespace Dive
{
	class Graphics;

	class Texture : public Resource
	{
	public:
		Texture() = default;
		virtual ~Texture() override;

		ID3D11ShaderResourceView* GetShaderResourceView() const { return m_srv.Get(); }

		uint32_t GetWidth() const { return m_width; }
		uint32_t GetHeight() const { return m_height; }

	protected:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;

		std::unique_ptr<DirectX::ScratchImage> m_scratchImage;
		std::unique_ptr<DirectX::TexMetadata> m_metaData;

		uint32_t m_width = 0;
		uint32_t m_height = 0;
	};
}