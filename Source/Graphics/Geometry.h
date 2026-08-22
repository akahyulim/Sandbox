#pragma once
#include <DirectXMath.h>
#include <cstdint>
#include <vector>

namespace Dive
{
	struct SimpleVertex
	{
		SimpleVertex() = default;
		SimpleVertex(const DirectX::XMFLOAT3& pos)
			: Position(pos)
		{}

		DirectX::XMFLOAT3 Position{ 0.0f, 0.0f, 0.0f };
	};

	struct StaticVertex
	{
		StaticVertex() = default;
		StaticVertex(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT2& tex, const DirectX::XMFLOAT3& nor,
			const DirectX::XMFLOAT3& tan, const DirectX::XMFLOAT3& biNormal)
			: Position(pos), TexCoord(tex), Normal(nor), Tangent(tan), BiNormal(biNormal) {
		}

		DirectX::XMFLOAT3 Position{ 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT2 TexCoord{ 0.0f, 0.0f };
		DirectX::XMFLOAT3 Normal{ 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 Tangent{ 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 BiNormal{ 0.0f, 0.0f, 0.0f };
	};

	struct SkinnedVertex
	{
		SkinnedVertex() = default;
		SkinnedVertex(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT2& tex, const DirectX::XMFLOAT3& nor,
			const DirectX::XMFLOAT3& tan, const DirectX::XMFLOAT3& biNormal,
			const DirectX::XMFLOAT4& wgt, const DirectX::XMUINT4& idx)
			: Position(pos), TexCoord(tex), Normal(nor), Tangent(tan), BiNormal(biNormal)
			, Weights(wgt), Indices(idx) {
		}

		DirectX::XMFLOAT3 Position{ 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT2 TexCoord{ 0.0f, 0.0f };
		DirectX::XMFLOAT3 Normal{ 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 Tangent{ 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 BiNormal{ 0.0f, 0.0f, 0.0f };

		DirectX::XMFLOAT4 Weights{ 0.0f, 0.0f, 0.0f, 0.0f };
		DirectX::XMUINT4 Indices{ 0, 0, 0, 0 };
	};

	struct StaticGeometryData
	{
		std::vector<StaticVertex> vertices;
		std::vector<uint32_t> indices;
	};

	struct SkinnedGeometryData
	{
		std::vector<SkinnedVertex> vertices;
		std::vector<uint32_t> indices;
	};
}