#pragma once
#include <fstream>
#include <vector>
#include <cstdint>
#include "Mesh.h"
#include "Graphics/Vertex.h"

namespace Dive
{
	class VertexBuffer;
	class IndexBuffer;
	class IResourceDesc;

	class SkinnedMesh : public Mesh
	{
	public:
		SkinnedMesh() : Mesh(eMeshType::Skinned) {}

		virtual bool Create() override;
		virtual void Release() override;

		bool Create(GraphicsDevice* device, std::shared_ptr<IResourceDesc> desc) override;

		std::vector<SkinnedVertex> GetVertices() const { return m_vertices; }
		void SetVertices(std::vector<SkinnedVertex> vertices) { m_vertices = vertices; }
		
		uint32_t GetVertexCount() const override { return static_cast<uint32_t>(m_vertices.size()); }

		eResourceType GetType() const override { return eResourceType::SkinnedMesh; }
		static constexpr eResourceType StaticType() { return eResourceType::SkinnedMesh; }

	protected:
		bool Serialize(std::ofstream& fout) const override;
		bool Deserialize(std::ifstream& fin) override;

	private:
		std::vector<SkinnedVertex> m_vertices;
	};
}