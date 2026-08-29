#include "pch.h"
#include "Preset.h"
#include "Graphics/Geometry.h"

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

	StaticGeometryData Preset::GenerateTriangle()
	{
		StaticGeometryData data{};

		data.vertices.resize(3);

		// 좌하단
		data.vertices[0] = {
			XMFLOAT3(-TRIANGLE_SIZE / 2.0f, -TRIANGLE_SIZE / 2.0f, 0.0f),   // Position
			XMFLOAT2(0.0f, 1.0f),                                           // UV
			XMFLOAT3(0.0f, 0.0f, -1.0f),                                    // Normal
			XMFLOAT3(1.0f, 0.0f, 0.0f),                                     // Tangent
			XMFLOAT3(0.0f, 1.0f, 0.0f)                                      // Binormal (Bitangent)
		};

		// 상단
		data.vertices[1] = {
			XMFLOAT3(0.0f, TRIANGLE_SIZE / 2.0f, 0.0f),
			XMFLOAT2(0.5f, 0.0f),
			XMFLOAT3(0.0f, 0.0f, -1.0f),
			XMFLOAT3(1.0f, 0.0f, 0.0f),
			XMFLOAT3(0.0f, 1.0f, 0.0f)
		};

		// 우하단
		data.vertices[2] = {
			XMFLOAT3(TRIANGLE_SIZE / 2.0f, -TRIANGLE_SIZE / 2.0f, 0.0f),
			XMFLOAT2(1.0f, 1.0f),
			XMFLOAT3(0.0f, 0.0f, -1.0f),
			XMFLOAT3(1.0f, 0.0f, 0.0f),
			XMFLOAT3(0.0f, 1.0f, 0.0f)
		};

		data.indices.resize(3);
		data.indices[0] = 0;
		data.indices[1] = 1;
		data.indices[2] = 2;

		return data;
	}

	StaticGeometryData Preset::GenerateQuad()
	{
		StaticGeometryData data;

		data.vertices.resize(4);

		// 좌하단
		data.vertices[0] = {
			XMFLOAT3(-QUAD_SIZE / 2.0f, -QUAD_SIZE / 2.0f, 0.0f),   // Position
			XMFLOAT2(0.0f, 1.0f),                                   // UV
			XMFLOAT3(0.0f, 0.0f, -1.0f),                            // Normal
			XMFLOAT3(1.0f, 0.0f, 0.0f),                             // Tangent
			XMFLOAT3(0.0f, 1.0f, 0.0f)                              // Binormal
		};

		// 좌상단
		data.vertices[1] = {
			XMFLOAT3(-QUAD_SIZE / 2.0f, QUAD_SIZE / 2.0f, 0.0f),
			XMFLOAT2(0.0f, 0.0f),
			XMFLOAT3(0.0f, 0.0f, -1.0f),
			XMFLOAT3(1.0f, 0.0f, 0.0f),
			XMFLOAT3(0.0f, 1.0f, 0.0f)
		};

		// 우하단
		data.vertices[2] = {
			XMFLOAT3(QUAD_SIZE / 2.0f, -QUAD_SIZE / 2.0f, 0.0f),
			XMFLOAT2(1.0f, 1.0f),
			XMFLOAT3(0.0f, 0.0f, -1.0f),
			XMFLOAT3(1.0f, 0.0f, 0.0f),
			XMFLOAT3(0.0f, 1.0f, 0.0f)
		};

		// 우상단
		data.vertices[3] = {
			XMFLOAT3(QUAD_SIZE / 2.0f, QUAD_SIZE / 2.0f, 0.0f),
			XMFLOAT2(1.0f, 0.0f),
			XMFLOAT3(0.0f, 0.0f, -1.0f),
			XMFLOAT3(1.0f, 0.0f, 0.0f),
			XMFLOAT3(0.0f, 1.0f, 0.0f)
		};

		data.indices.resize(6);
		data.indices[0] = 0;	data.indices[1] = 1;	data.indices[2] = 2;
		data.indices[3] = 2;	data.indices[4] = 1;	data.indices[5] = 3;

		return data;
	}

	StaticGeometryData Preset::GeneratePlane()
	{
		StaticGeometryData data;

		// 10x10 격자
		const uint32_t subdivisions = 10;
		const uint32_t vertexCount = (subdivisions + 1) * (subdivisions + 1);
		const uint32_t indexCount = subdivisions * subdivisions * 6;

		data.vertices.resize(vertexCount);
		data.indices.resize(indexCount);

		float startX = -PLANE_SIZE / 2.0f;
		float startZ = PLANE_SIZE / 2.0f;
		float cellSize = PLANE_SIZE / static_cast<float>(subdivisions);

		uint32_t index = 0;
		for (uint32_t row = 0; row <= subdivisions; ++row)
		{
			for (uint32_t col = 0; col <= subdivisions; ++col)
			{
				data.vertices[index].Position.x = startX + (static_cast<float>(col) * cellSize);
				data.vertices[index].Position.y = 0.0f;
				data.vertices[index].Position.z = startZ - (static_cast<float>(row) * cellSize);

				data.vertices[index].TexCoord.x = static_cast<float>(col) / static_cast<float>(subdivisions);
				data.vertices[index].TexCoord.y = static_cast<float>(row) / static_cast<float>(subdivisions);

				data.vertices[index].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
				data.vertices[index].Tangent = XMFLOAT3(1.0f, 0.0f, 0.0f);
				data.vertices[index].BiNormal = XMFLOAT3(0.0f, 0.0f, 1.0f);

				index++;
			}
		}

		index = 0;
		for (uint32_t row = 0; row < subdivisions; ++row)
		{
			for (uint32_t col = 0; col < subdivisions; ++col)
			{
				uint32_t topLeft = row * (subdivisions + 1) + col;
				uint32_t topRight = topLeft + 1;
				uint32_t bottomLeft = (row + 1) * (subdivisions + 1) + col;
				uint32_t bottomRight = bottomLeft + 1;

				data.indices[index++] = topLeft;
				data.indices[index++] = topRight;
				data.indices[index++] = bottomLeft;

				data.indices[index++] = topRight;
				data.indices[index++] = bottomRight;
				data.indices[index++] = bottomLeft;
			}
		}

		return data;
	}

	StaticGeometryData Preset::GenerateCube()
	{
		StaticGeometryData data;

		// 전면
		data.vertices.emplace_back(XMFLOAT3(-CUBE_SIZE / 2, -CUBE_SIZE / 2, -CUBE_SIZE / 2), XMFLOAT2(0, 1), XMFLOAT3(0, 0, -1), XMFLOAT3(1, 0, 0), XMFLOAT3(0, -1, 0));
		data.vertices.emplace_back(XMFLOAT3(-CUBE_SIZE / 2, CUBE_SIZE / 2, -CUBE_SIZE / 2), XMFLOAT2(0, 0), XMFLOAT3(0, 0, -1), XMFLOAT3(1, 0, 0), XMFLOAT3(0, -1, 0));
		data.vertices.emplace_back(XMFLOAT3(CUBE_SIZE / 2, -CUBE_SIZE / 2, -CUBE_SIZE / 2), XMFLOAT2(1, 1), XMFLOAT3(0, 0, -1), XMFLOAT3(1, 0, 0), XMFLOAT3(0, -1, 0));
		data.vertices.emplace_back(XMFLOAT3(CUBE_SIZE / 2, CUBE_SIZE / 2, -CUBE_SIZE / 2), XMFLOAT2(1, 0), XMFLOAT3(0, 0, -1), XMFLOAT3(1, 0, 0), XMFLOAT3(0, -1, 0));

		// 후면
		data.vertices.emplace_back(XMFLOAT3(CUBE_SIZE / 2, -CUBE_SIZE / 2, CUBE_SIZE / 2), XMFLOAT2(0, 1), XMFLOAT3(0, 0, 1), XMFLOAT3(-1, 0, 0), XMFLOAT3(0, -1, 0));
		data.vertices.emplace_back(XMFLOAT3(CUBE_SIZE / 2, CUBE_SIZE / 2, CUBE_SIZE / 2), XMFLOAT2(0, 0), XMFLOAT3(0, 0, 1), XMFLOAT3(-1, 0, 0), XMFLOAT3(0, -1, 0));
		data.vertices.emplace_back(XMFLOAT3(-CUBE_SIZE / 2, -CUBE_SIZE / 2, CUBE_SIZE / 2), XMFLOAT2(1, 1), XMFLOAT3(0, 0, 1), XMFLOAT3(-1, 0, 0), XMFLOAT3(0, -1, 0));
		data.vertices.emplace_back(XMFLOAT3(-CUBE_SIZE / 2, CUBE_SIZE / 2, CUBE_SIZE / 2), XMFLOAT2(1, 0), XMFLOAT3(0, 0, 1), XMFLOAT3(-1, 0, 0), XMFLOAT3(0, -1, 0));

		// 위
		data.vertices.emplace_back(XMFLOAT3(-CUBE_SIZE / 2, CUBE_SIZE / 2, -CUBE_SIZE / 2), XMFLOAT2(0, 1), XMFLOAT3(0, 1, 0), XMFLOAT3(1, 0, 0), XMFLOAT3(0, 0, -1));
		data.vertices.emplace_back(XMFLOAT3(-CUBE_SIZE / 2, CUBE_SIZE / 2, CUBE_SIZE / 2), XMFLOAT2(0, 0), XMFLOAT3(0, 1, 0), XMFLOAT3(1, 0, 0), XMFLOAT3(0, 0, -1));
		data.vertices.emplace_back(XMFLOAT3(CUBE_SIZE / 2, CUBE_SIZE / 2, -CUBE_SIZE / 2), XMFLOAT2(1, 1), XMFLOAT3(0, 1, 0), XMFLOAT3(1, 0, 0), XMFLOAT3(0, 0, -1));
		data.vertices.emplace_back(XMFLOAT3(CUBE_SIZE / 2, CUBE_SIZE / 2, CUBE_SIZE / 2), XMFLOAT2(1, 0), XMFLOAT3(0, 1, 0), XMFLOAT3(1, 0, 0), XMFLOAT3(0, 0, -1));

		// 아래
		data.vertices.emplace_back(XMFLOAT3(-CUBE_SIZE / 2, -CUBE_SIZE / 2, CUBE_SIZE / 2), XMFLOAT2(0, 1), XMFLOAT3(0, -1, 0), XMFLOAT3(1, 0, 0), XMFLOAT3(0, 0, 1));
		data.vertices.emplace_back(XMFLOAT3(-CUBE_SIZE / 2, -CUBE_SIZE / 2, -CUBE_SIZE / 2), XMFLOAT2(0, 0), XMFLOAT3(0, -1, 0), XMFLOAT3(1, 0, 0), XMFLOAT3(0, 0, 1));
		data.vertices.emplace_back(XMFLOAT3(CUBE_SIZE / 2, -CUBE_SIZE / 2, CUBE_SIZE / 2), XMFLOAT2(1, 1), XMFLOAT3(0, -1, 0), XMFLOAT3(1, 0, 0), XMFLOAT3(0, 0, 1));
		data.vertices.emplace_back(XMFLOAT3(CUBE_SIZE / 2, -CUBE_SIZE / 2, -CUBE_SIZE / 2), XMFLOAT2(1, 0), XMFLOAT3(0, -1, 0), XMFLOAT3(1, 0, 0), XMFLOAT3(0, 0, 1));

		// 왼쪽
		data.vertices.emplace_back(XMFLOAT3(-CUBE_SIZE / 2, -CUBE_SIZE / 2, CUBE_SIZE / 2), XMFLOAT2(0, 1), XMFLOAT3(-1, 0, 0), XMFLOAT3(0, 0, -1), XMFLOAT3(0, -1, 0));
		data.vertices.emplace_back(XMFLOAT3(-CUBE_SIZE / 2, CUBE_SIZE / 2, CUBE_SIZE / 2), XMFLOAT2(0, 0), XMFLOAT3(-1, 0, 0), XMFLOAT3(0, 0, -1), XMFLOAT3(0, -1, 0));
		data.vertices.emplace_back(XMFLOAT3(-CUBE_SIZE / 2, -CUBE_SIZE / 2, -CUBE_SIZE / 2), XMFLOAT2(1, 1), XMFLOAT3(-1, 0, 0), XMFLOAT3(0, 0, -1), XMFLOAT3(0, -1, 0));
		data.vertices.emplace_back(XMFLOAT3(-CUBE_SIZE / 2, CUBE_SIZE / 2, -CUBE_SIZE / 2), XMFLOAT2(1, 0), XMFLOAT3(-1, 0, 0), XMFLOAT3(0, 0, -1), XMFLOAT3(0, -1, 0));

		// 오른쪽
		data.vertices.emplace_back(XMFLOAT3(CUBE_SIZE / 2, -CUBE_SIZE / 2, -CUBE_SIZE / 2), XMFLOAT2(0, 1), XMFLOAT3(1, 0, 0), XMFLOAT3(0, 0, 1), XMFLOAT3(0, -1, 0));
		data.vertices.emplace_back(XMFLOAT3(CUBE_SIZE / 2, CUBE_SIZE / 2, -CUBE_SIZE / 2), XMFLOAT2(0, 0), XMFLOAT3(1, 0, 0), XMFLOAT3(0, 0, 1), XMFLOAT3(0, -1, 0));
		data.vertices.emplace_back(XMFLOAT3(CUBE_SIZE / 2, -CUBE_SIZE / 2, CUBE_SIZE / 2), XMFLOAT2(1, 1), XMFLOAT3(1, 0, 0), XMFLOAT3(0, 0, 1), XMFLOAT3(0, -1, 0));
		data.vertices.emplace_back(XMFLOAT3(CUBE_SIZE / 2, CUBE_SIZE / 2, CUBE_SIZE / 2), XMFLOAT2(1, 0), XMFLOAT3(1, 0, 0), XMFLOAT3(0, 0, 1), XMFLOAT3(0, -1, 0));

		// 전면
		data.indices.emplace_back(0);	data.indices.emplace_back(1);	data.indices.emplace_back(2);
		data.indices.emplace_back(2);	data.indices.emplace_back(1);	data.indices.emplace_back(3);

		// 후면
		data.indices.emplace_back(4);	data.indices.emplace_back(5);	data.indices.emplace_back(6);
		data.indices.emplace_back(6);	data.indices.emplace_back(5);	data.indices.emplace_back(7);

		// 위
		data.indices.emplace_back(8);	data.indices.emplace_back(9);	data.indices.emplace_back(10);
		data.indices.emplace_back(10);	data.indices.emplace_back(9);	data.indices.emplace_back(11);

		// 아래
		data.indices.emplace_back(12);	data.indices.emplace_back(13);	data.indices.emplace_back(14);
		data.indices.emplace_back(14);	data.indices.emplace_back(13);	data.indices.emplace_back(15);

		// 왼쪽
		data.indices.emplace_back(16);	data.indices.emplace_back(17);	data.indices.emplace_back(18);
		data.indices.emplace_back(18);	data.indices.emplace_back(17);	data.indices.emplace_back(19);

		// 오른쪽
		data.indices.emplace_back(20);	data.indices.emplace_back(21);	data.indices.emplace_back(22);
		data.indices.emplace_back(22);	data.indices.emplace_back(21);	data.indices.emplace_back(23);

		return data;
	}

	StaticGeometryData Preset::GenerateSphere()
	{
		StaticGeometryData data;

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

				data.vertices.emplace_back(position, XMFLOAT2(u, v), normal, tangent, binormal);
			}
		}

		// --- 인덱스 연결 ---
		// 💡 미리 5,400개의 메모리 공간을 예약해두어 push_back 속도를 극한으로 끌어올립니다.
		data.indices.reserve(latitudeBands * longitudeBands * 6);

		auto connectBands = [&](int start, int bands) {
			for (int lat = 0; lat < bands; ++lat)
			{
				for (int lon = 0; lon < longitudeBands; ++lon)
				{
					int i0 = start + lat * (longitudeBands + 1) + lon;
					int i1 = i0 + 1;
					int i2 = i0 + (longitudeBands + 1);
					int i3 = i2 + 1;

					data.indices.push_back(i0);
					data.indices.push_back(i1);
					data.indices.push_back(i2);
					data.indices.push_back(i1);
					data.indices.push_back(i3);
					data.indices.push_back(i2);
				}
			}
			};

		int startIndex = 0;
		connectBands(startIndex, latitudeBands);

		return data;
	}

	StaticGeometryData Preset::GenerateCapsule()
	{
		StaticGeometryData data;

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

				data.vertices.emplace_back(position, XMFLOAT2(u, v), normal, tangent, binormal);
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

				data.vertices.emplace_back(position, XMFLOAT2(u, v), normal, tangent, binormal);
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

				data.vertices.emplace_back(position, XMFLOAT2(u, v), normal, tangent, binormal);
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

					data.indices.push_back(i0);
					data.indices.push_back(i1);
					data.indices.push_back(i2);
					data.indices.push_back(i1);
					data.indices.push_back(i3);
					data.indices.push_back(i2);
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

					data.indices.push_back(i0);
					data.indices.push_back(i2);
					data.indices.push_back(i1);
					data.indices.push_back(i1);
					data.indices.push_back(i2);
					data.indices.push_back(i3);
				}
			}
			};

		data.indices.reserve((latitudeBands * 2 + 1) * longitudeBands * 6);

		// 인덱스 연결
		int topStart = 0;
		int cylinderStart = topStart + (latitudeBands + 1) * (longitudeBands + 1);
		int bottomStart = cylinderStart + 2 * (longitudeBands + 1);

		connectBands(topStart, latitudeBands);               // 위쪽 반구
		connectBands(cylinderStart, 1);                      // 실린더
		connectBandsReversed(bottomStart, latitudeBands);    // 아래쪽 반구 (반전)

		return data;
	}
}