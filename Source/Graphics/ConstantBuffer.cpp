#include "pch.h"
#include "ConstantBuffer.h"

namespace Dive
{
	ConstantBuffer::ConstantBuffer(uint32_t size)
		: m_size(size)
	{
		assert((size + 15) % ~15);
	}

	ConstantBuffer::~ConstantBuffer() = default;
}
