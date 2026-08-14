#pragma once
#include <memory>
#include <vector>
#include <d3d11.h>

#include "Graphics/Geometry.h"

namespace Dive
{
	class Graphics;
	class VertexBuffer;
	class IndexBuffer;

	class Mesh
	{
	public:
		Mesh() = delete;
		Mesh(ID3D11Device* device, const std::vector<StaticVertex>& vertices, const std::vector<uint32_t>& indices);
		Mesh(const Mesh&) = delete;
		Mesh(Mesh&&) noexcept = default;
		~Mesh();

		Mesh& operator=(const Mesh&) = delete;
		Mesh& operator=(Mesh&&) noexcept = default;

		void Bind(Graphics* graphics) const;

		uint32_t GetVertexCount() const;
		uint32_t GetIndexCount() const;

	private:
		std::unique_ptr<VertexBuffer> m_vb;
		std::unique_ptr<IndexBuffer> m_ib;
	};
}