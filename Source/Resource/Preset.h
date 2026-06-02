#pragma once
#include <cstdint>
#include <vector>

namespace Dive
{
	struct StaticVertex;

	enum class ePresetType
	{
		Triangle,
		Quad,
		Plane,
		Cube,
		Sphere,
		Capsule,
		None
	};

	// 유니티의 경우 읽기 전용 Material Default가 적용된다.
	// 근데 내 구현에선 Material을 이 곳에서 설정하지 않는다.
	class Preset
	{
	public:
		static void GenerateTriangle(std::vector<StaticVertex>& outVertices, std::vector<uint32_t>& outIndices);
		static void GenerateQuad(std::vector<StaticVertex>& outVertices, std::vector<uint32_t>& outIndices);
		static void GeneratePlane(std::vector<StaticVertex>& outVertices, std::vector<uint32_t>& outIndices);
		static void GenerateCube(std::vector<StaticVertex>& outVertices, std::vector<uint32_t>& outIndices);
		static void GenerateSphere(std::vector<StaticVertex>& outVertices, std::vector<uint32_t>& outIndices);
		static void GenerateCapsule(std::vector<StaticVertex>& outVertices, std::vector<uint32_t>& outIndices);
	};
}