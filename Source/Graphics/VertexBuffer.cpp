#include "pch.h"
#include "VertexBuffer.h"

namespace Dive
{
	VertexBuffer::VertexBuffer(uint32_t stride, uint32_t count)
		: m_stride(stride)
		, m_count(count)
	{
	}
	
	VertexBuffer::~VertexBuffer()
	{
		m_buffer.Reset();
	}
}