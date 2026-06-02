#pragma once
#include <fstream>
#include <vector>
#include <cstdint>

#include "Mesh.h"
#include "Graphics/Vertex.h"

namespace Dive
{
	class Shader;
	class Graphics;

	class StaticMesh : public Mesh
	{
	public:
		StaticMesh(): Mesh(eMeshType::Static) {}

		virtual bool Deserialize(const std::vector<uint8_t>& fileBuffer, std::wstring_view extension) override;

		virtual bool Create(Graphics* graphics) override;

		void SetVertices(const std::vector<StaticVertex>& vertices) { m_vertices = vertices; }

		std::vector<StaticVertex> GetVertices() { return m_vertices; }
		std::vector<uint32_t> GetIndices() { return m_indices; }

		uint32_t GetVertexCount() const { return static_cast<uint32_t>(m_vertices.size()); }
		uint32_t GetIndexCount() const { return static_cast<uint32_t>(m_indices.size()); }

		eResourceType GetType() const override { return eResourceType::StaticMesh; }
		static constexpr eResourceType StaticType() { return eResourceType::StaticMesh; }

	protected:

	private:
		std::vector<StaticVertex> m_vertices;
	};
}
