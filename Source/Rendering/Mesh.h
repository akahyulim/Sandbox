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
		Mesh(Graphics* graphics, const std::vector<StaticVertex>& vertices, const std::vector<uint32_t>& indices);
		Mesh(Graphics* graphics, const std::vector<SkinnedVertex>& vertices, const std::vector<uint32_t>& indices);
		Mesh(const Mesh&) = delete;
		Mesh(Mesh&&) noexcept = default;
		~Mesh();

		Mesh& operator=(const Mesh&) = delete;
		Mesh& operator=(Mesh&&) noexcept = default;

		void Bind(Graphics* graphics) const;

		uint32_t GetVertexCount() const;
		uint32_t GetIndexCount() const;

		bool IsSkinned() const { return m_isSkinned; }

	private:
		void createIndexBuffer(Graphics* graphics, const std::vector<uint32_t>& indices);

	private:
		std::unique_ptr<VertexBuffer> m_vb;
		std::unique_ptr<IndexBuffer> m_ib;

		bool m_isSkinned = false;
	};
}