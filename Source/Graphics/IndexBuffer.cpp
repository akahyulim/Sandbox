#include "pch.h"
#include "IndexBuffer.h"

namespace Dive
{
	IndexBuffer::IndexBuffer(eFormat format, uint32_t count)
		: m_format(format)
		, m_count(count)
	{
	}

	IndexBuffer::~IndexBuffer()
	{
		m_buffer.Reset();
	}

	uint32_t IndexBuffer::GetStride() const
	{
		return (m_format == eFormat::R16_UINT) ? 
			static_cast<uint32_t>(sizeof(uint16_t)) : static_cast<uint32_t>(sizeof(uint32_t));
	}
}