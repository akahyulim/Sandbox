#include "pch.h"
#include "Preset.h"
#include "Graphics/Vertex.h"

using namespace DirectX;

namespace Dive
{
	constexpr float TRIANGLE_SIZE = 1.0f;
	constexpr float CUBE_SIZE = 1.0f;
	constexpr float QUAD_SIZE = 1.0f;
	constexpr float PLANE_SIZE = 10.0f;
	constexpr float SPHERE_RADIUS = 0.5f;
	constexpr float CAPSULE_RADIUS = 0.5f;
	constexpr float CAPSULE_HEIGHT = 1.0f;

	void Preset::GenerateTriangle(std::vector<StaticVertex>& outVertices, std::vector<uint32_t>& outIndices)
	{
		outVertices.clear();
		outIndices.clear();

		outVertices.resize(3);

		// 좌하단
		outVertices[0] = {
			XMFLOAT3(-TRIANGLE_SIZE / 2.0f, -TRIANGLE_SIZE / 2.0f, 0.0f),   // Position
			XMFLOAT2(0.0f, 1.0f),                                           // UV
			XMFLOAT3(0.0f, 0.0f, -1.0f),                                    // Normal
			XMFLOAT3(1.0f, 0.0f, 0.0f),                                     // Tangent
			XMFLOAT3(0.0f, 1.0f, 0.0f)                                      // Binormal (Bitangent)
		};

		// 상단
		outVertices[1] = {
			XMFLOAT3(0.0f, TRIANGLE_SIZE / 2.0f, 0.0f),
			XMFLOAT2(0.5f, 0.0f),
			XMFLOAT3(0.0f, 0.0f, -1.0f),
			XMFLOAT3(1.0f, 0.0f, 0.0f),
			XMFLOAT3(0.0f, 1.0f, 0.0f)
		};

		// 우하단
		outVertices[2] = {
			XMFLOAT3(TRIANGLE_SIZE / 2.0f, -TRIANGLE_SIZE / 2.0f, 0.0f),
			XMFLOAT2(1.0f, 1.0f),
			XMFLOAT3(0.0f, 0.0f, -1.0f),
			XMFLOAT3(1.0f, 0.0f, 0.0f),
			XMFLOAT3(0.0f, 1.0f, 0.0f)
		};

		outIndices.resize(3);
		outIndices[0] = 0;	
		outIndices[1] = 1;	
		outIndices[2] = 2;
	}

	void Preset::GenerateQuad(std::vector<StaticVertex>& outVertices, std::vector<uint32_t>& outIndices)
	{
		outVertices.clear();
		outIndices.clear();

		outVertices.resize(4);

		// 좌하단
		outVertices[0] = {
			XMFLOAT3(-QUAD_SIZE / 2.0f, -QUAD_SIZE / 2.0f, 0.0f),   // Position
			XMFLOAT2(0.0f, 1.0f),                                   // UV
			XMFLOAT3(0.0f, 0.0f, -1.0f),                            // Normal
			XMFLOAT3(1.0f, 0.0f, 0.0f),                             // Tangent
			XMFLOAT3(0.0f, 1.0f, 0.0f)                              // Binormal
		};

		// 좌상단
		outVertices[1] = {
			XMFLOAT3(-QUAD_SIZE / 2.0f, QUAD_SIZE / 2.0f, 0.0f),
			XMFLOAT2(0.0f, 0.0f),
			XMFLOAT3(0.0f, 0.0f, -1.0f),
			XMFLOAT3(1.0f, 0.0f, 0.0f),
			XMFLOAT3(0.0f, 1.0f, 0.0f)
		};

		// 우하단
		outVertices[2] = {
			XMFLOAT3(QUAD_SIZE / 2.0f, -QUAD_SIZE / 2.0f, 0.0f),
			XMFLOAT2(1.0f, 1.0f),
			XMFLOAT3(0.0f, 0.0f, -1.0f),
			XMFLOAT3(1.0f, 0.0f, 0.0f),
			XMFLOAT3(0.0f, 1.0f, 0.0f)
		};

		// 우상단
		outVertices[3] = {
			XMFLOAT3(QUAD_SIZE / 2.0f, QUAD_SIZE / 2.0f, 0.0f),
			XMFLOAT2(1.0f, 0.0f),
			XMFLOAT3(0.0f, 0.0f, -1.0f),
			XMFLOAT3(1.0f, 0.0f, 0.0f),
			XMFLOAT3(0.0f, 1.0f, 0.0f)
		};

		outIndices.resize(6);
		outIndices[0] = 0;	outIndices[1] = 1;	outIndices[2] = 2;
		outIndices[3] = 2;	outIndices[4] = 1;	outIndices[5] = 3;
	}

	void Preset::GeneratePlane(std::vector<StaticVertex>& outVertices, std::vector<uint32_t>& outIndices)
	{
		outVertices.clear();
		outIndices.clear();

		outVertices.resize(4);

		// 좌후면
		outVertices[0] = {
			XMFLOAT3(-PLANE_SIZE / 2.0f, 0.0f, PLANE_SIZE / 2.0f),   // Position
			XMFLOAT2(0.0f, 0.0f),                                    // UV
			XMFLOAT3(0.0f, 1.0f, 0.0f),                              // Normal
			XMFLOAT3(1.0f, 0.0f, 0.0f),                              // Tangent
			XMFLOAT3(0.0f, 0.0f, -1.0f)                              // Binormal
		};

		// 우후면
		outVertices[1] = {
			XMFLOAT3(PLANE_SIZE / 2.0f, 0.0f, PLANE_SIZE / 2.0f),
			XMFLOAT2(1.0f, 0.0f),
			XMFLOAT3(0.0f, 1.0f, 0.0f),
			XMFLOAT3(1.0f, 0.0f, 0.0f),
			XMFLOAT3(0.0f, 0.0f, -1.0f)
		};

		// 좌전면
		outVertices[2] = {
			XMFLOAT3(-PLANE_SIZE / 2.0f, 0.0f, -PLANE_SIZE / 2.0f),
			XMFLOAT2(0.0f, 1.0f),
			XMFLOAT3(0.0f, 1.0f, 0.0f),
			XMFLOAT3(1.0f, 0.0f, 0.0f),
			XMFLOAT3(0.0f, 0.0f, -1.0f)
		};

		// 우전면
		outVertices[3] = {
			XMFLOAT3(PLANE_SIZE / 2.0f, 0.0f, -PLANE_SIZE / 2.0f),
			XMFLOAT2(1.0f, 1.0f),
			XMFLOAT3(0.0f, 1.0f, 0.0f),
			XMFLOAT3(1.0f, 0.0f, 0.0f),
			XMFLOAT3(0.0f, 0.0f, -1.0f)
		};

		outIndices.resize(6);
		outIndices[0] = 0;	outIndices[1] = 1;	outIndices[2] = 2;
		outIndices[3] = 1;	outIndices[4] = 3;	outIndices[5] = 2;
	}

	void Preset::GenerateCube(std::vector<StaticVertex>& outVertices, std::vector<uint32_t>& outIndices)
	{
		outVertices.clear();
		outIndices.clear();

		// 전면
		outVertices.emplace_back(XMFLOAT3(-CUBE_SIZE / 2, -CUBE_SIZE / 2, -CUBE_SIZE / 2), XMFLOAT2(0, 1), XMFLOAT3(0, 0, -1), XMFLOAT3(1, 0, 0), XMFLOAT3(0, -1, 0));
		outVertices.emplace_back(XMFLOAT3(-CUBE_SIZE / 2, CUBE_SIZE / 2, -CUBE_SIZE / 2), XMFLOAT2(0, 0), XMFLOAT3(0, 0, -1), XMFLOAT3(1, 0, 0), XMFLOAT3(0, -1, 0));
		outVertices.emplace_back(XMFLOAT3(CUBE_SIZE / 2, -CUBE_SIZE / 2, -CUBE_SIZE / 2), XMFLOAT2(1, 1), XMFLOAT3(0, 0, -1), XMFLOAT3(1, 0, 0), XMFLOAT3(0, -1, 0));
		outVertices.emplace_back(XMFLOAT3(CUBE_SIZE / 2, CUBE_SIZE / 2, -CUBE_SIZE / 2), XMFLOAT2(1, 0), XMFLOAT3(0, 0, -1), XMFLOAT3(1, 0, 0), XMFLOAT3(0, -1, 0));

		// 후면
		outVertices.emplace_back(XMFLOAT3(CUBE_SIZE / 2, -CUBE_SIZE / 2, CUBE_SIZE / 2), XMFLOAT2(0, 1), XMFLOAT3(0, 0, 1), XMFLOAT3(-1, 0, 0), XMFLOAT3(0, -1, 0));
		outVertices.emplace_back(XMFLOAT3(CUBE_SIZE / 2, CUBE_SIZE / 2, CUBE_SIZE / 2), XMFLOAT2(0, 0), XMFLOAT3(0, 0, 1), XMFLOAT3(-1, 0, 0), XMFLOAT3(0, -1, 0));
		outVertices.emplace_back(XMFLOAT3(-CUBE_SIZE / 2, -CUBE_SIZE / 2, CUBE_SIZE / 2), XMFLOAT2(1, 1), XMFLOAT3(0, 0, 1), XMFLOAT3(-1, 0, 0), XMFLOAT3(0, -1, 0));
		outVertices.emplace_back(XMFLOAT3(-CUBE_SIZE / 2, CUBE_SIZE / 2, CUBE_SIZE / 2), XMFLOAT2(1, 0), XMFLOAT3(0, 0, 1), XMFLOAT3(-1, 0, 0), XMFLOAT3(0, -1, 0));

		// 위
		outVertices.emplace_back(XMFLOAT3(-CUBE_SIZE / 2, CUBE_SIZE / 2, -CUBE_SIZE / 2), XMFLOAT2(0, 1), XMFLOAT3(0, 1, 0), XMFLOAT3(1, 0, 0), XMFLOAT3(0, 0, -1));
		outVertices.emplace_back(XMFLOAT3(-CUBE_SIZE / 2, CUBE_SIZE / 2, CUBE_SIZE / 2), XMFLOAT2(0, 0), XMFLOAT3(0, 1, 0), XMFLOAT3(1, 0, 0), XMFLOAT3(0, 0, -1));
		outVertices.emplace_back(XMFLOAT3(CUBE_SIZE / 2, CUBE_SIZE / 2, -CUBE_SIZE / 2), XMFLOAT2(1, 1), XMFLOAT3(0, 1, 0), XMFLOAT3(1, 0, 0), XMFLOAT3(0, 0, -1));
		outVertices.emplace_back(XMFLOAT3(CUBE_SIZE / 2, CUBE_SIZE / 2, CUBE_SIZE / 2), XMFLOAT2(1, 0), XMFLOAT3(0, 1, 0), XMFLOAT3(1, 0, 0), XMFLOAT3(0, 0, -1));

		// 아래
		outVertices.emplace_back(XMFLOAT3(-CUBE_SIZE / 2, -CUBE_SIZE / 2, CUBE_SIZE / 2), XMFLOAT2(0, 1), XMFLOAT3(0, -1, 0), XMFLOAT3(1, 0, 0), XMFLOAT3(0, 0, 1));
		outVertices.emplace_back(XMFLOAT3(-CUBE_SIZE / 2, -CUBE_SIZE / 2, -CUBE_SIZE / 2), XMFLOAT2(0, 0), XMFLOAT3(0, -1, 0), XMFLOAT3(1, 0, 0), XMFLOAT3(0, 0, 1));
		outVertices.emplace_back(XMFLOAT3(CUBE_SIZE / 2, -CUBE_SIZE / 2, CUBE_SIZE / 2), XMFLOAT2(1, 1), XMFLOAT3(0, -1, 0), XMFLOAT3(1, 0, 0), XMFLOAT3(0, 0, 1));
		outVertices.emplace_back(XMFLOAT3(CUBE_SIZE / 2, -CUBE_SIZE / 2, -CUBE_SIZE / 2), XMFLOAT2(1, 0), XMFLOAT3(0, -1, 0), XMFLOAT3(1, 0, 0), XMFLOAT3(0, 0, 1));

		// 왼쪽
		outVertices.emplace_back(XMFLOAT3(-CUBE_SIZE / 2, -CUBE_SIZE / 2, CUBE_SIZE / 2), XMFLOAT2(0, 1), XMFLOAT3(-1, 0, 0), XMFLOAT3(0, 0, -1), XMFLOAT3(0, -1, 0));
		outVertices.emplace_back(XMFLOAT3(-CUBE_SIZE / 2, CUBE_SIZE / 2, CUBE_SIZE / 2), XMFLOAT2(0, 0), XMFLOAT3(-1, 0, 0), XMFLOAT3(0, 0, -1), XMFLOAT3(0, -1, 0));
		outVertices.emplace_back(XMFLOAT3(-CUBE_SIZE / 2, -CUBE_SIZE / 2, -CUBE_SIZE / 2), XMFLOAT2(1, 1), XMFLOAT3(-1, 0, 0), XMFLOAT3(0, 0, -1), XMFLOAT3(0, -1, 0));
		outVertices.emplace_back(XMFLOAT3(-CUBE_SIZE / 2, CUBE_SIZE / 2, -CUBE_SIZE / 2), XMFLOAT2(1, 0), XMFLOAT3(-1, 0, 0), XMFLOAT3(0, 0, -1), XMFLOAT3(0, -1, 0));

		// 오른쪽
		outVertices.emplace_back(XMFLOAT3(CUBE_SIZE / 2, -CUBE_SIZE / 2, -CUBE_SIZE / 2), XMFLOAT2(0, 1), XMFLOAT3(1, 0, 0), XMFLOAT3(0, 0, 1), XMFLOAT3(0, -1, 0));
		outVertices.emplace_back(XMFLOAT3(CUBE_SIZE / 2, CUBE_SIZE / 2, -CUBE_SIZE / 2), XMFLOAT2(0, 0), XMFLOAT3(1, 0, 0), XMFLOAT3(0, 0, 1), XMFLOAT3(0, -1, 0));
		outVertices.emplace_back(XMFLOAT3(CUBE_SIZE / 2, -CUBE_SIZE / 2, CUBE_SIZE / 2), XMFLOAT2(1, 1), XMFLOAT3(1, 0, 0), XMFLOAT3(0, 0, 1), XMFLOAT3(0, -1, 0));
		outVertices.emplace_back(XMFLOAT3(CUBE_SIZE / 2, CUBE_SIZE / 2, CUBE_SIZE / 2), XMFLOAT2(1, 0), XMFLOAT3(1, 0, 0), XMFLOAT3(0, 0, 1), XMFLOAT3(0, -1, 0));

		// 전면 (정상)
		outIndices.emplace_back(0);	outIndices.emplace_back(1);	outIndices.emplace_back(2);
		outIndices.emplace_back(2);	outIndices.emplace_back(1);	outIndices.emplace_back(3);

		// 후면 (⭕ 수정)
		outIndices.emplace_back(4);	outIndices.emplace_back(6);	outIndices.emplace_back(5);
		outIndices.emplace_back(5);	outIndices.emplace_back(6);	outIndices.emplace_back(7);

		// 위 (정상)
		outIndices.emplace_back(8);	outIndices.emplace_back(9);	outIndices.emplace_back(10);
		outIndices.emplace_back(10);	outIndices.emplace_back(9);	outIndices.emplace_back(11);

		// 아래 (⭕ 수정)
		outIndices.emplace_back(12);	outIndices.emplace_back(14);	outIndices.emplace_back(13);
		outIndices.emplace_back(13);	outIndices.emplace_back(14);	outIndices.emplace_back(15);

		// 왼쪽 (⭕ 수정)
		outIndices.emplace_back(16);	outIndices.emplace_back(18);	outIndices.emplace_back(17);
		outIndices.emplace_back(17);	outIndices.emplace_back(18);	outIndices.emplace_back(19);

		// 오른쪽 (정상)
		outIndices.emplace_back(20);	outIndices.emplace_back(21);	outIndices.emplace_back(22);
		outIndices.emplace_back(22);	outIndices.emplace_back(21);	outIndices.emplace_back(23);

		/*
		// 전면
		outIndices.emplace_back(0);	outIndices.emplace_back(1);	outIndices.emplace_back(2);
		outIndices.emplace_back(2);	outIndices.emplace_back(1);	outIndices.emplace_back(3);

		// 후면
		outIndices.emplace_back(4);	outIndices.emplace_back(5);	outIndices.emplace_back(6);
		outIndices.emplace_back(6);	outIndices.emplace_back(5);	outIndices.emplace_back(7);

		// 위
		outIndices.emplace_back(8);	outIndices.emplace_back(9);	outIndices.emplace_back(10);
		outIndices.emplace_back(10);	outIndices.emplace_back(9);	outIndices.emplace_back(11);

		// 아래
		outIndices.emplace_back(12);	outIndices.emplace_back(13);	outIndices.emplace_back(14);
		outIndices.emplace_back(14);	outIndices.emplace_back(13);	outIndices.emplace_back(15);

		// 왼쪽
		outIndices.emplace_back(16);	outIndices.emplace_back(17);	outIndices.emplace_back(18);
		outIndices.emplace_back(18);	outIndices.emplace_back(17);	outIndices.emplace_back(19);

		// 오른쪽
		outIndices.emplace_back(20);	outIndices.emplace_back(21);	outIndices.emplace_back(22);
		outIndices.emplace_back(22);	outIndices.emplace_back(21);	outIndices.emplace_back(23);
		*/
	}

	void Preset::GenerateSphere(std::vector<StaticVertex>& outVertices, std::vector<uint32_t>& outIndices)
	{
		outVertices.clear();
		outIndices.clear();

		constexpr int latitudeBands = 30;
		constexpr int longitudeBands = 30;
		constexpr float radius = SPHERE_RADIUS;

		XMVECTOR upVec = XMVectorSet(0, 1, 0, 0);
		XMFLOAT3 center(0.0f, 0.0f, 0.0f);

		// 정점 생성
		for (int lat = 0; lat <= latitudeBands; ++lat)
		{
			float theta = XM_PI * (lat / static_cast<float>(latitudeBands)); // 0 ~ π
			float sinTheta = sinf(theta);
			float cosTheta = cosf(theta);

			for (int lon = 0; lon <= longitudeBands; ++lon)
			{
				float phi = 2.0f * XM_PI * (lon / static_cast<float>(longitudeBands)); // 0 ~ 2π
				float sinPhi = sinf(phi);
				float cosPhi = cosf(phi);

				XMFLOAT3 offset(cosPhi * sinTheta, cosTheta, sinPhi * sinTheta);
				XMFLOAT3 position(radius * offset.x, radius * offset.y, radius * offset.z);

				float u = lon / static_cast<float>(longitudeBands);
				float v = lat / static_cast<float>(latitudeBands);

				XMVECTOR posVec = XMLoadFloat3(&position);
				XMVECTOR centerVec = XMLoadFloat3(&center);
				XMVECTOR normalVec = XMVector3Normalize(posVec - centerVec);
				XMFLOAT3 normal;
				XMStoreFloat3(&normal, normalVec);

				XMVECTOR tangentVec = XMVector3Normalize(XMVector3Cross(normalVec, upVec));
				XMFLOAT3 tangent;
				XMStoreFloat3(&tangent, tangentVec);

				XMVECTOR binormalVec = XMVector3Normalize(XMVector3Cross(normalVec, tangentVec));
				XMFLOAT3 binormal;
				XMStoreFloat3(&binormal, binormalVec);

				outVertices.emplace_back(position, XMFLOAT2(u, v), normal, tangent, binormal);
			}
		}

		// --- 인덱스 연결 ---
		// 💡 미리 5,400개의 메모리 공간을 예약해두어 push_back 속도를 극한으로 끌어올립니다.
		outIndices.reserve(latitudeBands * longitudeBands * 6);

		auto connectBands = [&](int start, int bands) {
			for (int lat = 0; lat < bands; ++lat)
			{
				for (int lon = 0; lon < longitudeBands; ++lon)
				{
					int i0 = start + lat * (longitudeBands + 1) + lon;
					int i1 = i0 + 1;
					int i2 = i0 + (longitudeBands + 1);
					int i3 = i2 + 1;

					outIndices.push_back(i0);
					outIndices.push_back(i1);
					outIndices.push_back(i2);
					outIndices.push_back(i1);
					outIndices.push_back(i3);
					outIndices.push_back(i2);
				}
			}
			};

		int startIndex = 0;
		connectBands(startIndex, latitudeBands);
		/*
		// 인덱스 연결
		auto connectBands = [&](int start, int bands) {
			for (int lat = 0; lat < bands; ++lat)
			{
				for (int lon = 0; lon < longitudeBands; ++lon)
				{
					int i0 = start + lat * (longitudeBands + 1) + lon;
					int i1 = i0 + 1;
					int i2 = i0 + (longitudeBands + 1);
					int i3 = i2 + 1;

					outIndices.push_back(i0);
					outIndices.push_back(i1);
					outIndices.push_back(i2);
					outIndices.push_back(i1);
					outIndices.push_back(i3);
					outIndices.push_back(i2);
				}
			}
			};
		
		int startIndex = 0;
		connectBands(startIndex, latitudeBands);
		*/
	}

	void Preset::GenerateCapsule(std::vector<StaticVertex>& outVertices, std::vector<uint32_t>& outIndices)
	{
		outVertices.clear();
		outIndices.clear();

		constexpr int latitudeBands = 30;
		constexpr int longitudeBands = 30;
		constexpr float radius = CAPSULE_RADIUS;
		constexpr float height = CAPSULE_HEIGHT;

		XMVECTOR upVec = XMVectorSet(0, 1, 0, 0);
		XMFLOAT3 centerTop(0.0f, height / 2.0f, 0.0f);
		XMFLOAT3 centerBottom(0.0f, -height / 2.0f, 0.0f);

		// 위쪽 반구
		for (int lat = 0; lat <= latitudeBands; ++lat)
		{
			float theta = XM_PI / 2.0f * (lat / static_cast<float>(latitudeBands));
			float sinTheta = sinf(theta);
			float cosTheta = cosf(theta);

			for (int lon = 0; lon <= longitudeBands; ++lon)
			{
				float phi = 2.0f * XM_PI * (lon / static_cast<float>(longitudeBands));
				float sinPhi = sinf(phi);
				float cosPhi = cosf(phi);

				XMFLOAT3 offset(cosPhi * sinTheta, cosTheta, sinPhi * sinTheta);
				XMFLOAT3 position(
					radius * offset.x,
					centerTop.y + radius * offset.y,
					radius * offset.z);

				float u = lon / static_cast<float>(longitudeBands);
				float v = lat / static_cast<float>(latitudeBands) * 0.25f;

				XMVECTOR posVec = XMLoadFloat3(&position);
				XMVECTOR centerVec = XMLoadFloat3(&centerTop);
				XMVECTOR normalVec = XMVector3Normalize(posVec - centerVec);
				XMFLOAT3 normal;
				XMStoreFloat3(&normal, normalVec);

				XMVECTOR tangentVec = XMVector3Normalize(XMVector3Cross(normalVec, upVec));
				XMFLOAT3 tangent;
				XMStoreFloat3(&tangent, tangentVec);

				XMVECTOR binormalVec = XMVector3Normalize(XMVector3Cross(normalVec, tangentVec));
				XMFLOAT3 binormal;
				XMStoreFloat3(&binormal, binormalVec);

				outVertices.emplace_back(position, XMFLOAT2(u, v), normal, tangent, binormal);
			}
		}

		// 실린더
		for (int yStep = 0; yStep <= 1; ++yStep)
		{
			float y = centerTop.y - yStep * height;

			for (int lon = 0; lon <= longitudeBands; ++lon)
			{
				float phi = 2.0f * XM_PI * (lon / static_cast<float>(longitudeBands));
				float sinPhi = sinf(phi);
				float cosPhi = cosf(phi);

				XMFLOAT3 position(radius * cosPhi, y, radius * sinPhi);

				float u = lon / static_cast<float>(longitudeBands);
				float v = 0.25f + yStep * 0.5f;

				XMFLOAT3 normal(cosPhi, 0.0f, sinPhi);

				XMVECTOR normalVec = XMVector3Normalize(XMLoadFloat3(&normal));
				XMVECTOR tangentVec = XMVector3Normalize(XMVector3Cross(normalVec, upVec));
				XMFLOAT3 tangent;
				XMStoreFloat3(&tangent, tangentVec);

				XMVECTOR binormalVec = XMVector3Normalize(XMVector3Cross(normalVec, tangentVec));
				XMFLOAT3 binormal;
				XMStoreFloat3(&binormal, binormalVec);

				outVertices.emplace_back(position, XMFLOAT2(u, v), normal, tangent, binormal);
			}
		}

		// 아래쪽 반구
		for (int lat = 0; lat <= latitudeBands; ++lat)
		{
			float theta = XM_PI / 2.0f * (lat / static_cast<float>(latitudeBands));
			float sinTheta = sinf(theta);
			float cosTheta = cosf(theta);

			for (int lon = 0; lon <= longitudeBands; ++lon)
			{
				float phi = 2.0f * XM_PI * (lon / static_cast<float>(longitudeBands));
				float sinPhi = sinf(phi);
				float cosPhi = cosf(phi);

				XMFLOAT3 offset(cosPhi * sinTheta, -cosTheta, sinPhi * sinTheta);
				XMFLOAT3 position(
					radius * offset.x,
					centerBottom.y + radius * offset.y,
					radius * offset.z);

				float u = lon / static_cast<float>(longitudeBands);
				float v = 0.75f + lat / static_cast<float>(latitudeBands) * 0.25f;

				XMVECTOR posVec = XMLoadFloat3(&position);
				XMVECTOR centerVec = XMLoadFloat3(&centerBottom);
				XMVECTOR normalVec = XMVector3Normalize(posVec - centerVec);
				XMFLOAT3 normal;
				XMStoreFloat3(&normal, normalVec);

				XMVECTOR tangentVec = XMVector3Normalize(XMVector3Cross(normalVec, upVec));
				XMFLOAT3 tangent;
				XMStoreFloat3(&tangent, tangentVec);

				XMVECTOR binormalVec = XMVector3Normalize(XMVector3Cross(normalVec, tangentVec));
				XMFLOAT3 binormal;
				XMStoreFloat3(&binormal, binormalVec);

				outVertices.emplace_back(position, XMFLOAT2(u, v), normal, tangent, binormal);
			}
		}

		// 인덱스 연결 함수들
		auto connectBands = [&](int start, int bands) {
			for (int lat = 0; lat < bands; ++lat)
			{
				for (int lon = 0; lon < longitudeBands; ++lon)
				{
					int i0 = start + lat * (longitudeBands + 1) + lon;
					int i1 = i0 + 1;
					int i2 = i0 + (longitudeBands + 1);
					int i3 = i2 + 1;

					outIndices.push_back(i0);
					outIndices.push_back(i1);
					outIndices.push_back(i2);
					outIndices.push_back(i1);
					outIndices.push_back(i3);
					outIndices.push_back(i2);
				}
			}
			};

		auto connectBandsReversed = [&](int start, int bands) {
			for (int lat = 0; lat < bands; ++lat)
			{
				for (int lon = 0; lon < longitudeBands; ++lon)
				{
					int i0 = start + lat * (longitudeBands + 1) + lon;
					int i1 = i0 + 1;
					int i2 = i0 + (longitudeBands + 1);
					int i3 = i2 + 1;

					outIndices.push_back(i0);
					outIndices.push_back(i2);
					outIndices.push_back(i1);
					outIndices.push_back(i1);
					outIndices.push_back(i2);
					outIndices.push_back(i3);
				}
			}
			};

		outIndices.reserve((latitudeBands * 2 + 1)* longitudeBands * 6);

		// 인덱스 연결
		int topStart = 0;
		int cylinderStart = topStart + (latitudeBands + 1) * (longitudeBands + 1);
		int bottomStart = cylinderStart + 2 * (longitudeBands + 1);

		connectBands(topStart, latitudeBands);               // 위쪽 반구
		connectBands(cylinderStart, 1);                      // 실린더
		connectBandsReversed(bottomStart, latitudeBands);    // 아래쪽 반구 (반전)
	}
}