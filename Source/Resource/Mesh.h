#pragma once
#include <DirectXMath.h>
#include <fstream>
#include <d3d11.h>
#include <filesystem>
#include <vector>
#include <memory>
#include <cstdint>
#include <algorithm>

#include "Core/Types.h"
#include "Resource.h"

#include "Graphics/VertexBuffer.h"
#include "Graphics/IndexBuffer.h"

namespace Dive
{
	class Graphics;

	class Mesh : public Resource
	{
		friend class Graphics;

	public:
		Mesh(eMeshType type) : m_meshType(type) {}
		virtual ~Mesh();

		void Bind(Graphics* graphics);

		eMeshType GetMeshType() const { return m_meshType; }
		
		ePrimitiveTopology GetTopology() const { return m_primitiveTopology; }
		void SetPrimitiveTopology(ePrimitiveTopology primitiveTopology) { m_primitiveTopology = primitiveTopology; }

		uint32_t GetVertexCount() const { return m_vertexBuffer ? m_vertexBuffer->GetCount() : 0; }
		uint32_t GetIndexCount() const { return m_indexBuffer ? m_indexBuffer->GetCount() : 0; }
	
		VertexBuffer* GetVertexBuffer() const { return m_vertexBuffer.get(); }
		IndexBuffer* GetIndexBuffer() const { return m_indexBuffer.get(); }

		Bounds GetBounds() const { return m_bounds; }

	protected:
		eMeshType m_meshType = eMeshType::None;

		ePrimitiveTopology m_primitiveTopology = ePrimitiveTopology::TriangleList;

		std::unique_ptr<VertexBuffer> m_vertexBuffer;
		std::unique_ptr<IndexBuffer> m_indexBuffer;

		Bounds m_bounds;
	};
}