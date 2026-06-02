#pragma once
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <filesystem>
#include <directxmath.h>

#include "Core/Types.h"

namespace Dive
{
	enum class eMapType : uint8_t
	{
		Diffuse,
		Normal,
		Count
	};

	enum class eRenderingMode
	{
		Opqaue,
		Transparent
	};

	struct alignas(16) MaterialParams
	{
		DirectX::XMFLOAT4 diffuseColor{ 0.0f, 0.0f, 0.0f, 1.0f };
		DirectX::XMFLOAT2 tiling{ 1.0f, 1.0f };
		DirectX::XMFLOAT2 offset{ 0.0f, 0.0f };

		uint32_t flags{ 0 };			// 0: hasDiffuse, 1: hasNormal, 2: hasAlpha, 3: isTransparent
		uint32_t renderingMode{ 0 };	// 0: opaque, 1: transparent
		uint32_t padding[2]{};
	};

	struct IResourceData
	{
		virtual ~IResourceData() = default;
		virtual void Clear() {}
	};

	struct MaterialData : public IResourceData
	{

	};

	// ================================================================================================================

	// Desc보단 Data더 마음에 든다.
	struct IResourceDesc
	{
		virtual ~IResourceDesc() = default;
	};

	struct TextureDesc : public IResourceDesc
	{
		std::vector<uint8_t> pixels;
		uint32_t width, height;
		uint32_t format;
	};

	// 제미나이는 ShaderDesc도 예시로 보여줬다.
	// 이때 std::vector<uint8_t> blob을 데이터로 선언했다.

	struct StaticMeshDesc : public IResourceDesc
	{
		std::string name;
		std::vector<StaticVertex> vertices;
		std::vector<uint32_t> indices;
	};

	struct MaterialDesc : public IResourceDesc
	{
		std::string name;
		std::string shaderName;
		uint32_t renderingMode;

		std::array<std::filesystem::path, static_cast<size_t>(eMapType::Count)> mapPaths;

		MaterialParams params;
	};
}