#pragma once
#include <cstdint>
#include <wrl/client.h>

struct ID3D11Buffer;

namespace Dive
{
	class VertexBuffer
	{
	public:
		VertexBuffer() = delete;
		VertexBuffer(uint32_t stride, uint32_t count);
		~VertexBuffer();

		ID3D11Buffer* GetRawBuffer() const { return m_buffer.Get(); }
		ID3D11Buffer** GetAddressOf() { return m_buffer.GetAddressOf(); }

		uint32_t GetStride() const { return m_stride; }
		uint32_t GetCount() const { return m_count; }

		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer& operator=(const VertexBuffer&) = delete;

	private:
		const uint32_t m_stride;
		const uint32_t m_count;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;
	};
}