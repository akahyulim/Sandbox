#pragma once
#include <cstdint>
#include <wrl/client.h>
#include <d3d11.h>

#include "Core/Types.h"

namespace Dive
{
	class Graphics;

	class IndexBuffer
	{
	public:
		IndexBuffer() = delete;
		IndexBuffer(Graphics* graphics, eFormat format, uint32_t count, const void* data);
		IndexBuffer(eFormat format, uint32_t count);
		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer(IndexBuffer&&) = default;
		~IndexBuffer();

		IndexBuffer& operator=(const IndexBuffer&) = delete;
		IndexBuffer& operator=(IndexBuffer&&) = default;

		ID3D11Buffer* GetRawBuffer() const { return m_buffer.Get(); }
		ID3D11Buffer** GetAddressOf() { return m_buffer.GetAddressOf(); }
		
		eFormat GetFormat() const { return m_format; }
		uint32_t GetStride() const;
		uint32_t GetCount() const { return m_count; }

	private:
		uint32_t m_count = 0;
		eFormat m_format = eFormat::Unknown;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;
	};
}