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
	public:
		Mesh(eMeshType type) : m_meshType(type) {}
		virtual ~Mesh();

		void Bind(Graphics* graphics);

		eMeshType GetMeshType() const { return m_meshType; }
		
		ePrimitiveTopology GetTopology() const { return m_primitiveTopology; }
		void SetPrimitiveTopology(ePrimitiveTopology primitiveTopology) { m_primitiveTopology = primitiveTopology; }

		virtual uint32_t GetVertexCount() const = 0;

		std::vector<uint32_t> GetIndices() { return m_indices; }
		void SetIndices(const std::vector<uint32_t>& indices) { m_indices = indices; }
		uint32_t GetIndexCount() const { return static_cast<uint32_t>(m_indices.size()); }
	
		VertexBuffer* GetVertexBuffer() const { return m_vertexBuffer.get(); }
		IndexBuffer* GetIndexBuffer() const { return m_indexBuffer.get(); }

		Bounds GetBounds() const { return m_bounds; }

	protected:
		eMeshType m_meshType = eMeshType::None;

		ePrimitiveTopology m_primitiveTopology = ePrimitiveTopology::TriangleList;

		std::vector<uint32_t> m_indices;

		std::unique_ptr<VertexBuffer> m_vertexBuffer;
		std::unique_ptr<IndexBuffer> m_indexBuffer;

		Bounds m_bounds;
	};
}