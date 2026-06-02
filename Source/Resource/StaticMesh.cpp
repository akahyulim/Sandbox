#include "pch.h"
#include "StaticMesh.h"
#include "ResourceDesc.h"
#include "Graphics/Graphics.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/IndexBuffer.h"

using namespace DirectX;

namespace Dive
{
	bool StaticMesh::Deserialize(const std::vector<uint8_t>& fileBuffer, std::wstring_view extension)
	{
		return true;
	}

	bool StaticMesh::Create(Graphics* graphics)
	{
		if (graphics == nullptr)
			return false;

		// 정점 버퍼 생성
		if (m_vertices.empty())
		{
			spdlog::error("[::CreateBuffers] 정점이 존재하지 않아 실패");
			return false;
		}

		m_vertexBuffer = std::move(graphics->CreateVertexBuffer(
			static_cast<uint32_t>(sizeof(StaticVertex)),
			static_cast<uint32_t>(m_vertices.size()),
			m_vertices.data()
		));
		if(m_vertexBuffer == nullptr)
		{
			spdlog::error("[::CreateBuffers] 정점 버퍼 생성에 실패");
			return false;
		}

		// 인덱스 버퍼 생성
		if (!m_indices.empty())
		{
			bool use32bit = std::any_of(m_indices.begin(), m_indices.end(), [](uint32_t i) { return i > 65535; });
			eFormat format = use32bit ? eFormat::R32_UINT : eFormat::R16_UINT;
			
			std::vector<uint16_t> indices16;
			const void* indexData = nullptr;

			if (use32bit)
			{
				indexData = m_indices.data();
			}
			else
			{
				indices16.reserve(m_indices.size());
				for (uint32_t i : m_indices)
					indices16.push_back(static_cast<uint16_t>(i));
				indexData = indices16.data();
			}

			m_indexBuffer = std::move(graphics->CreateIndexBuffer(
				format,
				static_cast<uint32_t>(m_indices.size()),
				indexData
			));
			if(m_indexBuffer == nullptr)
			{
				spdlog::error("[::CreateBuffers] 인덱스 버퍼 생성에 실패");
				return false;
			}
		}

		{
			XMFLOAT3 min = { FLT_MAX, FLT_MAX, FLT_MAX };
			XMFLOAT3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

			for (const auto& vertex : m_vertices)
			{
				const XMFLOAT3& pos = vertex.Position;

				min.x = std::min(min.x, pos.x);
				min.y = std::min(min.y, pos.y);
				min.z = std::min(min.z, pos.z);

				max.x = std::max(max.x, pos.x);
				max.y = std::max(max.y, pos.y);
				max.z = std::max(max.z, pos.z);
			}

			m_bounds.center = {
				(min.x + max.x) * 0.5f,
				(min.y + max.y) * 0.5f,
				(min.z + max.z) * 0.5f
			};

			m_bounds.extents = {
				(max.x - min.x) * 0.5f,
				(max.y - min.y) * 0.5f,
				(max.z - min.z) * 0.5f
			};
		}

		return true;
	}
}
