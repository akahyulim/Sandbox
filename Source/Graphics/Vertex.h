#pragma once
#include <DirectXMath.h>
#include <cstdint>
#include <algorithm>

namespace Dive
{
	struct StaticVertex
	{
		StaticVertex() = default;
		StaticVertex(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT2& tex, const DirectX::XMFLOAT3& nor,
			const DirectX::XMFLOAT3& tan, const DirectX::XMFLOAT3& biNormal)
			: Position(pos)
			, TexCoord(tex)
			, Normal(nor)
			, Tangent(tan)
			, BiNormal(biNormal)
		{
		}

		DirectX::XMFLOAT3 Position{ 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT2 TexCoord{ 0.0f, 0.0f };
		DirectX::XMFLOAT3 Normal{ 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 Tangent{ 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 BiNormal{ 0.0f, 0.0f, 0.0f };
	};

	struct SkinnedVertex : StaticVertex
	{
		SkinnedVertex() = default;
		SkinnedVertex(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT2& tex, const DirectX::XMFLOAT3& nor,
			const DirectX::XMFLOAT3& tan, const DirectX::XMFLOAT3& biNormal,
			const DirectX::XMFLOAT4& wgt, const uint32_t bi[4])
			: StaticVertex(pos, tex, nor, tan, biNormal)
			, Weights(wgt)
		{
			std::copy(bi, bi + 4, Indices);
		}

		DirectX::XMFLOAT4 Weights{0.0f, 0.0f, 0.0f, 0.0f};
		uint32_t Indices[4]{0, 0, 0, 0};
	};
}