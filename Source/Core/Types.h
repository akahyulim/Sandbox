#pragma once
#include <algorithm>
#include <DirectXMath.h>

namespace Dive
{
	enum class eLogLevel : int
	{
		Trace = 0, Debug, Info, Warn, Err, Critical, Off
	};

	enum class ePrimitiveTopology
	{
		None,
		PointList,
		LineList,
		LineStrip,
		TriangleList,
		TriangleStrip
	};

	enum class eFormat
	{
		Unknown,

		R16_UINT,
		R32_UINT,
		R32_FLOAT,

		R32G32_FLOAT,				// uv
		R32G32B32_FLOAT,			// 위치, 법선
		R32G32B32A32_FLOAT,			// 색상, 탄젠트

		R8G8B8A8_UNORM,
		R8G8B8A8_UNORM_SRGB,
		D24_UNORM_S8_UINT			// 깊이 스텐실 버퍼용
	};

	enum class eRasterizerState
	{
		FillSolid_CullFront,
		FillSolid_CullBack,
		FillSolid_CullNone,
		Count
	};

	enum class eBlendState
	{
		AlphaEnabled,
		AlphaDisabled,
		Additive,
		Count
	};

	enum class eSamplerState
	{
		WrapLinear,
		ClampPoint,
		ClampLinear,
		Skybox,
		ShadowCompare,
		Count
	};

	enum class eDepthStencilState
	{
		DepthReadWrite,
		DepthReadWrite_StencilReadWrite,
		GBuffer,
		DepthDisabled,   // skydome에서 off용으로...
		ForwardLight,
		Transparent,
		Skybox,
		Count
	};

	enum class eInputLayout : uint8_t
	{
		None = 0,
		PN,
		PNT,
		Unlit,
		Lit,
		Skinned
	};

	struct Bounds
	{
		Bounds() = default;
		Bounds(DirectX::XMFLOAT3 center, DirectX::XMFLOAT3 extents)
		{
			this->center = center;
			this->extents = extents;
		}

		void Encapsulate(const Bounds& other)
		{
			// 현재 박스의 min/max
			DirectX::XMFLOAT3 minA = {
				center.x - extents.x,
				center.y - extents.y,
				center.z - extents.z
			};
			DirectX::XMFLOAT3 maxA = {
				center.x + extents.x,
				center.y + extents.y,
				center.z + extents.z
			};

			// 다른 박스의 min/max
			DirectX::XMFLOAT3 minB = {
				other.center.x - other.extents.x,
				other.center.y - other.extents.y,
				other.center.z - other.extents.z
			};
			DirectX::XMFLOAT3 maxB = {
				other.center.x + other.extents.x,
				other.center.y + other.extents.y,
				other.center.z + other.extents.z
			};

			// 병합된 min/max
			DirectX::XMFLOAT3 minCombined = {
				std::min(minA.x, minB.x),
				std::min(minA.y, minB.y),
				std::min(minA.z, minB.z)
			};
			DirectX::XMFLOAT3 maxCombined = {
				std::max(maxA.x, maxB.x),
				std::max(maxA.y, maxB.y),
				std::max(maxA.z, maxB.z)
			};

			// 새로운 center/extents 갱신
			center = {
				(minCombined.x + maxCombined.x) * 0.5f,
				(minCombined.y + maxCombined.y) * 0.5f,
				(minCombined.z + maxCombined.z) * 0.5f
			};
			extents = {
				(maxCombined.x - minCombined.x) * 0.5f,
				(maxCombined.y - minCombined.y) * 0.5f,
				(maxCombined.z - minCombined.z) * 0.5f
			};
		}

		void GetCorners(DirectX::XMFLOAT3* corners) const
		{
			float ex = extents.x;
			float ey = extents.y;
			float ez = extents.z;

			corners[0] = DirectX::XMFLOAT3(center.x - ex, center.y - ey, center.z - ez);	// 전면 좌하
			corners[1] = DirectX::XMFLOAT3(center.x - ex, center.y - ey, center.z + ez);	// 후면 좌하
			corners[2] = DirectX::XMFLOAT3(center.x - ex, center.y + ey, center.z - ez);	// 전면 좌상
			corners[3] = DirectX::XMFLOAT3(center.x - ex, center.y + ey, center.z + ez);	// 후면 좌상
			corners[4] = DirectX::XMFLOAT3(center.x + ex, center.y - ey, center.z - ez);	// 전면 우하
			corners[5] = DirectX::XMFLOAT3(center.x + ex, center.y - ey, center.z + ez);	// 후면 우하
			corners[6] = DirectX::XMFLOAT3(center.x + ex, center.y + ey, center.z - ez);	// 전면 우상
			corners[7] = DirectX::XMFLOAT3(center.x + ex, center.y + ey, center.z + ez);	// 후면 우상
		}

		DirectX::XMFLOAT3 GetMax() const
		{
			return { center.x + extents.x, center.y + extents.y, center.z + extents.z };
		}

		DirectX::XMFLOAT3 GetMin() const
		{
			return { center.x - extents.x, center.y - extents.y, center.z - extents.z };
		}

		DirectX::XMFLOAT3 center = { 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 extents = { 0.0f, 0.0f, 0.0f };
	};

	enum class eMeshType : uint8_t
	{
		Static,
		Skinned,
		None
	};

	enum class eLightType : uint32_t
	{
		Directional = 0,
		Point,
		Spot
	};

	struct Viewport
	{
		float topLeftX = 0.0f;
		float topLeftY = 0.0f;
		float width = 0.0f;
		float height = 0.0f;
		float minDepth = 0.0f;
		float maxDepth = 0.0f;

		bool operator==(const Viewport& other) const
		{
			return (topLeftX == other.topLeftX && topLeftY == other.topLeftY &&
				width == other.width && height == other.height &&
				minDepth == other.minDepth && maxDepth == other.maxDepth);
		}

		bool operator!=(const Viewport& other) const
		{
			return !(*this == other);
		}
	};

	struct Color
	{
		float r, g, b, a;

		Color() : r(0), g(0), b(0), a(1.0f) {}
		Color(float red, float green, float blue, float alpha = 1.0f) : r(red), g(green), b(blue), a(alpha) {}
		Color(const DirectX::XMFLOAT4& color) : r(color.x), g(color.y), b(color.z), a(color.w) {}

		operator const float* () const { return &r; }
		operator float* () { return &r; }

		Color& operator=(const DirectX::XMFLOAT4& other)
		{
			r = other.x; 
			g = other.y;
			b = other.z;
			a = other.w;

			return *this;
		}

		Color& operator=(DirectX::XMVECTOR other)
		{
			DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(this), other);
			return *this;
		}
		
		operator DirectX::XMFLOAT4() const { return DirectX::XMFLOAT4(r, g, b, a); }
		operator DirectX::XMVECTOR() const
		{
			DirectX::XMFLOAT4 temp = { r, g, b, a };
			return DirectX::XMLoadFloat4(&temp);
		}

		static const Color Black;
		static const Color White;
		static const Color Gray;
		static const Color MidnightGray;
		static const Color IndustrialBlue;
		static const Color ContrastCyan;
		static const Color SkyBlue;
		static const Color DeepSkyBlue;
		static const Color LightSkyBlue;
	};

	inline const Color Color::Black = Color(0.0f, 0.0f, 0.0f, 1.0f);
	inline const Color Color::White = Color(1.0f, 1.0f, 1.0f, 1.0f);
	inline const Color Color::Gray = Color(0.5f, 0.5f, 0.5f, 1.0f);
	inline const Color Color::MidnightGray = Color(0.15f, 0.16f, 0.18f, 1.0f);		// #26292E
	inline const Color Color::IndustrialBlue = Color(0.22f, 0.24f, 0.28f, 1.0f);	// #383D47
	inline const Color Color::ContrastCyan = Color(0.0f, 0.40f, 0.50f, 1.0f);		// #006680
	inline const Color Color::SkyBlue = Color(0.53f, 0.81f, 0.92f, 1.0f);			// #87CEEB
	inline const Color Color::DeepSkyBlue = Color(0.0f, 0.75f, 1.0f, 1.0f);			// #00BFFF
	inline const Color Color::LightSkyBlue = Color(0.53f, 0.81f, 0.98f, 1.0f);		// #87CEFA
}
