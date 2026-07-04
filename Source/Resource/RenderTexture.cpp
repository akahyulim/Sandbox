#include "pch.h"
#include "RenderTexture.h"

namespace Dive
{
	RenderTexture::RenderTexture(uint32_t width, uint32_t height, DXGI_FORMAT colorFormat, DXGI_FORMAT depthFormat)
		: Texture(width, height),
		m_colorFormat(colorFormat),
		m_depthFormat(depthFormat)
	{
	}

	RenderTexture::~RenderTexture() = default;
}