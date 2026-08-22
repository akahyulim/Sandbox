#include "pch.h"
#include "IndexBuffer.h"
#include "Graphics.h"

namespace Dive
{
	IndexBuffer::IndexBuffer(Graphics* graphics, eFormat format, uint32_t count, const void* data)
		: m_format(format),
		m_count(count)
	{
		assert(data != nullptr && "IndexBuffer (IMMUTABLE) requires initial data!");

		D3D11_BUFFER_DESC desc{};
		desc.ByteWidth = static_cast<UINT>(GetStride() * count);
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		desc.MiscFlags = 0;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA subData{};
		subData.pSysMem = data;

		auto hr = graphics->GetDevice()->CreateBuffer(&desc, &subData, m_buffer.GetAddressOf());
		if (FAILED(hr))
		{
			spdlog::error("IndexBuffer 생성 실패: {}", ErrorUtils::ToVerbose(hr));
		}
	}

	IndexBuffer::IndexBuffer(eFormat format, uint32_t count)
		: m_format(format)
		, m_count(count)
	{
	}

	IndexBuffer::~IndexBuffer() = default;

	uint32_t IndexBuffer::GetStride() const
	{
		if (m_format == eFormat::R16_UINT)
			return sizeof(uint16_t);
		else if (m_format == eFormat::R32_UINT)
			return sizeof(uint32_t);

		assert(false && "지원하지 않는 Index Buffer Format입니다!");
		return sizeof(uint32_t);
	}
}