#pragma once
#include "Resource.h"

#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <vector>
#include <memory>

namespace Dive
{
	class Graphics;

	class Texture : public Resource
	{
		friend class Graphics;

	public:
		Texture() = default;
		Texture(uint32_t width, uint32_t height);
		virtual ~Texture() override;

		ID3D11ShaderResourceView* GetShaderResourceView() const { return m_srv.Get(); }

		uint32_t GetWidth() const { return m_width; }
		uint32_t GetHeight() const { return m_height; }

	protected:
		uint32_t m_width = 0;
		uint32_t m_height = 0;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;
	};
}