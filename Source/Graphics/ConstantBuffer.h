#pragma once
#include <wrl/client.h>
#include <cstdint>
#include <d3d11.h>

namespace Dive
{
	class ConstantBuffer
	{
	public:
		ConstantBuffer() = delete;
		ConstantBuffer(uint32_t stride);
		~ConstantBuffer();

		ID3D11Buffer* GetRawBuffer() const { return m_buffer.Get(); }
		ID3D11Buffer** GetAddressOf() { return m_buffer.GetAddressOf(); }

		uint32_t GetSize() const { return m_size; }
		
		ConstantBuffer(const ConstantBuffer&) = delete;
		ConstantBuffer& operator=(const ConstantBuffer&) = delete;

	private:
		uint32_t m_size = 0;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;
	};
}
