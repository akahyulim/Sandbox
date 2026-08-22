#include "pch.h"
#include "Mesh.h"
#include "Graphics/Graphics.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/IndexBuffer.h"

namespace Dive
{
	Mesh::Mesh(Graphics* graphics, const std::vector<StaticVertex>& vertices, const std::vector<uint32_t>& indices)
	{
		assert(!vertices.empty());

		uint32_t stride = static_cast<uint32_t>(sizeof(StaticVertex));
		uint32_t count = static_cast<uint32_t>(vertices.size());

		m_vb = std::make_unique<VertexBuffer>(graphics, stride, count, vertices.data());
		
		if (!indices.empty())
		{
			bool use32bit = std::any_of(indices.begin(), indices.end(), [](uint32_t i) { return i > 65535; });
			eFormat format = use32bit ? eFormat::R32_UINT : eFormat::R16_UINT;

			std::vector<uint16_t> indices16;
			const void* indexData = nullptr;

			if (use32bit)
			{
				indexData = indices.data();
			}
			else
			{
				indices16.reserve(indices.size());
				for (uint32_t i : indices)
					indices16.push_back(static_cast<uint16_t>(i));
				indexData = indices16.data();
			}

			uint32_t count = static_cast<uint32_t>(indices.size());

			m_ib = std::make_unique<IndexBuffer>(graphics, format, count, indexData);
		}
	}

	Mesh::~Mesh() = default;

	void Mesh::Bind(Graphics* graphics) const
	{
		graphics->SetVertexBuffer(m_vb.get());

		if (m_ib)
			graphics->SetIndexBuffer(m_ib.get());
	}

	uint32_t Mesh::GetVertexCount() const
	{
		return m_vb ? m_vb->GetCount() : 0;
	}

	uint32_t Mesh::GetIndexCount() const
	{
		return m_ib ? m_ib->GetCount() : 0;
	}
}