#pragma once
#include "Texture.h"

namespace Dive
{
	class Texture2D : public Texture
	{
	public:
		Texture2D() = default;
		Texture2D(uint32_t width, uint32_t height);
		virtual ~Texture2D() override;

	private:
	private:
		// 이 두개도 사실은 필요없다?
		std::unique_ptr<DirectX::ScratchImage> m_scratchImage;
		std::unique_ptr<DirectX::TexMetadata> m_metaData;
	};
}