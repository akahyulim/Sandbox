#pragma once
#include <cstdint>
#include <wrl/client.h>

#include "Core/Types.h"

struct ID3D11Buffer;

namespace Dive
{
	class IndexBuffer
	{
	public:
		IndexBuffer() = delete;
		IndexBuffer(eFormat format, uint32_t count);
		~IndexBuffer();

		ID3D11Buffer* GetRawBuffer() const { return m_buffer.Get(); }
		ID3D11Buffer** GetAddressOf() { return m_buffer.GetAddressOf(); }
		
		eFormat GetFormat() const { return m_format; }
		uint32_t GetStride() const;
		uint32_t GetCount() const { return m_count; }

		IndexBuffer(const IndexBuffer&) = delete; 
		IndexBuffer& operator=(const IndexBuffer&) = delete;

	private:
		const uint32_t m_count;
		const eFormat m_format;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;
	};
}