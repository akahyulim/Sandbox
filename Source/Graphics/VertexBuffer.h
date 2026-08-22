#pragma once
#include <cstdint>
#include <wrl/client.h>
#include <d3d11.h>

namespace Dive
{
	class Graphics;

	class VertexBuffer
	{
	public:
		VertexBuffer() = delete;
		VertexBuffer(Graphics* graphics, uint32_t stride, uint32_t count, const void* data);
		VertexBuffer(uint32_t stride, uint32_t count);
		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer(VertexBuffer&&) = default;
		~VertexBuffer();

		VertexBuffer& operator=(const VertexBuffer&) = delete;
		VertexBuffer& operator=(VertexBuffer&&) = default;

		ID3D11Buffer* GetRawBuffer() const { return m_buffer.Get(); }
		ID3D11Buffer** GetAddressOf() { return m_buffer.GetAddressOf(); }

		uint32_t GetStride() const { return m_stride; }
		uint32_t GetCount() const { return m_count; }

	private:
		uint32_t m_stride = 0;
		uint32_t m_count = 0;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;
	};
}