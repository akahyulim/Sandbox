#include "pch.h"
#include "Mesh.h"
#include "Graphics/Graphics.h"

namespace Dive
{
	Mesh::~Mesh() = default;

	void Mesh::Bind(Graphics* graphics)
	{
		assert(graphics);

		graphics->BindVertexBuffer(m_vertexBuffer.get());
		if(m_indexBuffer)
			graphics->BindIndexBuffer(m_indexBuffer.get());
	}
}