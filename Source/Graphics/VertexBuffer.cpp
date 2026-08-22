#include "pch.h"
#include "VertexBuffer.h"
#include "Core/Common.h"
#include "Graphics.h"

namespace Dive
{
    VertexBuffer::VertexBuffer(Graphics* graphics, uint32_t stride, uint32_t count, const void* data)
        : m_stride(stride),
        m_count(count)
    {
        assert(data != nullptr && "VertexBuffer (IMMUTABLE) requires initial data!");

        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = static_cast<UINT>(stride * count);
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.MiscFlags = 0;
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA subData{};
        subData.pSysMem = data;

        auto hr = graphics->GetDevice()->CreateBuffer(&desc, &subData, m_buffer.GetAddressOf());
        if (FAILED(hr))
        {
            spdlog::error("VertexBuffer 생성 실패: {}", ErrorUtils::ToVerbose(hr));
        }
    }

	// 일단 살려두자.
	VertexBuffer::VertexBuffer(uint32_t stride, uint32_t count)
		: m_stride(stride)
		, m_count(count)
	{
	}
	
    VertexBuffer::~VertexBuffer() = default;
}