#include "stdafx.h"
#include "MeshRenderer.h"
#include "../Collision.h"
#include "../GameObject.h"
#include "../Components/Camera.h"
#include "../Components/Transform.h"
#include "API/Graphics.h"
#include "Graphics/GraphicsSystem.h"
#include "Rendering/RenderingSystem.h"
#include "Resource/ResourceSystem.h"

using namespace DirectX;

namespace Dive
{
	MeshRenderer::MeshRenderer(GameObject* owner, uint64_t id)
		: Component(owner, id)
	{
	}

	bool MeshRenderer::IsEqualTo(const Component* other) const
	{
		return false;
	}

	std::unique_ptr<Component> MeshRenderer::Clone(GameObject* owner) const
	{
		auto clone = std::make_unique<MeshRenderer>(owner);

		clone->m_staticMesh = m_staticMesh;
		// 머티리얼까지 얕은 복사로 가져가면 전부 하나의 머티리얼을 공유하게 된다.
		clone->m_material = m_material;

		return clone;
	}

	void MeshRenderer::Serialize(YAML::Emitter& out)
	{
		//if (!m_isDirty)	return;

		out << YAML::Key << "MeshRenderer" << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "ID" << YAML::Value << GetInstanceID();
		out << YAML::Key << "Mesh" << m_staticMesh->GetFilepath().string();
		out << YAML::Key << "Material" << m_material->GetFilepath().string();
		out << YAML::EndMap;

		m_isDirty = false;
	}

	// 기존 구현을 수정하면서 id 유지가 안되고 있다.
	void MeshRenderer::Deserialize(const YAML::Node& node)
	{
		if (GetInstanceID() != node["ID"].as<uint64_t>())
		{
			DV_LOG(MeshRenderer, eLogLevel::Err, "[::Deserialize] InstanceID 초기화에 실패하였습니다.");
			return;
		}

		m_staticMesh = Resources::Load<StaticMesh>(node["Mesh"].as<std::string>());
		m_material = Resources::Load<Material>(node["Material"].as<std::string>());

		m_isDirty = true;
	}

	void MeshRenderer::Update()
	{
		if (!m_owner || !m_owner->IsActiveSelf())
			return;
	}

	void MeshRenderer::Render()
	{
		// 1. cb update & bind
		// 2. material::bind
		m_staticMesh->Bind();

		Graphics::DrawIndexed(m_staticMesh->GetIndexCount());
	}

	bool MeshRenderer::IsVisible(const Frustum& frustum) const
	{
		const auto& boundingBox = GetBounds();
		return frustum.CheckAABB(boundingBox.center, boundingBox.extents);
	}

	bool MeshRenderer::Intersects(const Ray& ray, float* outDistance, DirectX::XMFLOAT3* outPoint, DirectX::XMFLOAT3* outNormal) const
	{
		const auto& boundingBox = GetBounds(); // 최신 Transform 반영

		// AABB와 Ray의 교차 검사
		if (!Collision::IntersectRayAABB(ray, boundingBox.center, boundingBox.extents, outDistance, outNormal))
			return false;

		// 교차 지점 계산
		if (outPoint && outDistance)
		{
			outPoint->x = ray.origin.x + ray.direction.x * (*outDistance);
			outPoint->y = ray.origin.y + ray.direction.y * (*outDistance);
			outPoint->z = ray.origin.z + ray.direction.z * (*outDistance);
		}

		return true;
	}

	void MeshRenderer::SetStaticMesh(std::shared_ptr<StaticMesh> staticMesh)
	{
		m_staticMesh = staticMesh;
	}

	void MeshRenderer::SetMaterial(std::shared_ptr<Material> material)
	{
		m_material = material;
	}

	// 프리셋, 프리팹의 경우 기본적으로 공유된 머티리얼을 사용
	// 사용자가 개별 머티리얼을 사용하고 싶을 때 기존 머티리얼을 복사한 추가 머티리얼을 만드는 것
	// 아직 구현을 마치지 않아.. 추후 수정이 필요함.
	void MeshRenderer::CloneMaterial()
	{
		//if (m_material)
		//	m_material = std::make_shared<Material>();	// 기존 m_material을 복사생성해야 한다.
	}

	// 8개의 정점을 월드 변환한 후 center와 extents 계산
	Bounds MeshRenderer::GetBounds() const
	{
		const auto& boundingBox = m_staticMesh->GetBounds();
		XMFLOAT3 corners[8]{};
		boundingBox.GetCorners(corners);
		
		XMVECTOR minVertex = XMVectorReplicate(FLT_MAX);
		XMVECTOR maxVertex = XMVectorReplicate(-FLT_MAX);
		auto worldMatrix = GetTransform()->GetTransformMatrix();
		for (int i = 0; i < 8; ++i)
		{
			XMVECTOR corner = XMLoadFloat3(&corners[i]);
			XMVECTOR worldCorner = XMVector3Transform(corner, worldMatrix);

			minVertex = XMVectorMin(minVertex, worldCorner);
			maxVertex = XMVectorMax(maxVertex, worldCorner);
		}

		XMVECTOR center = (minVertex + maxVertex) * 0.5f;
		XMVECTOR extents = (maxVertex - minVertex) * 0.5f;

		Bounds aabb{};
		XMStoreFloat3(&aabb.center, center);
		XMStoreFloat3(&aabb.extents, extents);

		return aabb;
	}
}
