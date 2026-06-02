#include "stdafx.h"
#include "Transform.h"
#include "../GameObject.h"
#include "../Scene.h"
//#include "Core/CoreSystem.h"
#include "Resource/ResourceSystem.h"

#include "Common/Math.h"

using namespace DirectX;

namespace Dive
{
	Transform::Transform(GameObject* owner, uint64_t id)
		: Component(owner, id)
	{
		XMStoreFloat4x4(&m_localTransform, XMMatrixIdentity());
		m_transform = m_localTransform;
	}

	Transform::~Transform()
	{
		if (m_parent)
			m_parent->RemoveChild(this);

		for (auto child : m_children)
			child->m_parent = nullptr;
	}

	bool Transform::IsEqualTo(const Component* other) const
	{
		auto target = dynamic_cast<const Transform*>(other);
		if (!target) return false;

		return Math::XMFLOAT3Equal(m_localPosition, target->m_localPosition) &&
			Math::XMFLOAT4Equal(m_localRotation, target->m_localRotation)&&
			Math::XMFLOAT3Equal(m_localScale, target->m_localScale);
	}

	std::unique_ptr<Component> Transform::Clone(GameObject* owner) const
	{
		auto clone = std::make_unique<Transform>(owner);
		
		clone->m_localPosition = m_localPosition;
		clone->m_localRotation = m_localRotation;
		clone->m_localScale = m_localScale;

		// 나머지는 캐시 데이터

		// 계층구조는 Prefab에서 설정
		clone->m_parent = nullptr;
		clone->m_children.clear();

		// 캐시 데이터 계산
		clone->m_isDirty = true;

		return clone;
	}

	void Transform::Serialize(YAML::Emitter& out)
	{
		// Transform의 특징인가.. Update에서 결국 false로 초기화된다.
		//if (!m_isDirty)
		//	return;
		
		out << YAML::Key << "Transform" << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "ID" << YAML::Value << GetInstanceID();
		out << YAML::Key << "Position" << YAML::Value << m_localPosition;
		out << YAML::Key << "Rotation" << YAML::Value << m_localRotation;
		out << YAML::Key << "Scale" << YAML::Value << m_localScale;
		if (auto parent = GetParent())	out << YAML::Key << "Parent" << YAML::Value << parent->GetInstanceID();
		out << YAML::EndMap;

		//m_isDirty = false;
	}

	void Transform::Deserialize(const YAML::Node& node)
	{
		SetInstanceID(node["ID"].as<uint64_t>());
		SetLocalPosition(node["Position"].as<XMFLOAT3>());
		SetLocalRotation(node["Rotation"].as<XMFLOAT4>());
		SetLocalScale(node["Scale"].as<XMFLOAT3>());

		// 계층구조 형성은 일단 직렬화를 끝낸 후로 미루기
		// Scene에 관려 메서드를 만들고 호출
		// 문제는 부모 Transform의 ID를 기반으로 형성해야 한다는 거다.

		//m_isDirty = true;
	}

	void Transform::Update()
	{
		if (!m_owner || !m_owner->IsActiveSelf())
			return;

		if (!m_isDirty)
			return;

		auto localMat =
			XMMatrixScalingFromVector(XMLoadFloat3(&m_localScale)) *
			XMMatrixRotationQuaternion(XMLoadFloat4(&m_localRotation)) *
			XMMatrixTranslationFromVector(XMLoadFloat3(&m_localPosition));

		XMStoreFloat4x4(&m_localTransform, localMat);

		if (m_parent)
		{
			auto worldMat = localMat * m_parent->GetTransformMatrix();
			XMStoreFloat4x4(&m_transform, worldMat);

			XMVECTOR scl, rot, pos;
			XMMatrixDecompose(&scl, &rot, &pos, worldMat);
			XMStoreFloat3(&m_position, pos);
			XMStoreFloat4(&m_rotation, rot);
			XMStoreFloat3(&m_scale, scl);
		}
		else
		{
			XMStoreFloat4x4(&m_transform, GetLocalTransformMatrix());

			m_position = m_localPosition;
			m_rotation = m_localRotation;
			m_scale = m_localScale;
		}

		m_isDirty = false;

		// 루트부터 시작해 하향식으로 자식들을 갱신 필요상태로 변경
		for (auto child : m_children)
			child->m_isDirty = true;
	}

	void Transform::SetPositionVector(const XMVECTOR& worldPos)
	{
		if (m_parent)
		{
			auto parentInv = XMMatrixInverse(nullptr, m_parent->GetTransformMatrix());
			auto localPos = XMVector3TransformCoord(worldPos, parentInv);
			XMStoreFloat3(&m_localPosition, localPos);
		}
		else
		{
			XMStoreFloat3(&m_localPosition, worldPos);
		}

		m_isDirty = true;
	}

	void Transform::SetPosition(const XMFLOAT3& worldPos)
	{
		SetPositionVector(XMLoadFloat3(&worldPos));
	}

	void Transform::SetLocalPositionVector(const XMVECTOR& localPos)
	{
		XMStoreFloat3(&m_localPosition, localPos);
		m_isDirty = true;
	}

	void Transform::SetLocalPosition(const XMFLOAT3& localPos)
	{
		m_localPosition = localPos;
		m_isDirty = true;
	}

	XMFLOAT3 Transform::GetRotationRadians() const
	{
		auto rotMatrix = XMMatrixRotationQuaternion(GetRotationVector());

		float pitch = std::atan2(rotMatrix.r[1].m128_f32[2], rotMatrix.r[2].m128_f32[2]);
		float yaw = std::atan2(-rotMatrix.r[0].m128_f32[2], std::sqrt(rotMatrix.r[0].m128_f32[0] * rotMatrix.r[0].m128_f32[0] + rotMatrix.r[0].m128_f32[1] * rotMatrix.r[0].m128_f32[1]));
		float roll = std::atan2(rotMatrix.r[0].m128_f32[1], rotMatrix.r[0].m128_f32[0]);

		return { pitch, yaw, roll };
	}

	XMFLOAT3 Transform::GetRotationDegrees() const
	{
		auto rotationRadians = GetRotationRadians();
		return XMFLOAT3(
			XMConvertToDegrees(rotationRadians.x),
			XMConvertToDegrees(rotationRadians.y),
			XMConvertToDegrees(rotationRadians.z)
		);
	}

	void Transform::SetRotationVector(const XMVECTOR& worldRot)
	{
		if (m_parent)
		{
			auto parentInvRot = XMQuaternionInverse(m_parent->GetRotationVector());
			auto localRot = XMQuaternionNormalize(XMQuaternionMultiply(parentInvRot, worldRot));
			XMStoreFloat4(&m_localRotation, localRot);
		}
		else
		{
			XMStoreFloat4(&m_localRotation, XMQuaternionNormalize(worldRot));
		}

		m_isDirty = true;
	}

	void Transform::SetRotation(const XMFLOAT4& worldRot)
	{
		SetRotationVector(XMLoadFloat4(&worldRot));
	}

	void Transform::SetRotationByRadians(const XMFLOAT3& radians)
	{
		SetRotationVector(XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&radians)));
	}

	void Transform::SetRotationByDegrees(const XMFLOAT3& degrees)
	{
		SetRotationByRadians({
			XMConvertToRadians(degrees.x),
			XMConvertToRadians(degrees.y),
			XMConvertToRadians(degrees.z)
			});
	}

	XMFLOAT3 Transform::GetLocalRotationRadians() const
	{
		auto rotMatrix = XMMatrixRotationQuaternion(GetLocalRotationVector());

		float pitch = std::atan2(rotMatrix.r[1].m128_f32[2], rotMatrix.r[2].m128_f32[2]);
		float yaw = std::atan2(-rotMatrix.r[0].m128_f32[2], std::sqrt(rotMatrix.r[0].m128_f32[0] * rotMatrix.r[0].m128_f32[0] + rotMatrix.r[0].m128_f32[1] * rotMatrix.r[0].m128_f32[1]));
		float roll = std::atan2(rotMatrix.r[0].m128_f32[1], rotMatrix.r[0].m128_f32[0]);

		return { pitch, yaw, roll };
	}

	void Transform::SetLocalRotationVector(const XMVECTOR& localRot)
	{
		XMStoreFloat4(&m_localRotation, XMQuaternionNormalize(localRot));
		m_localEulerDegrees = Math::QuaternionToDegrees(m_localRotation);
		m_isDirty = true;
	}

	void Transform::SetLocalRotation(const XMFLOAT4& localRot)
	{
		SetLocalRotationVector(XMLoadFloat4(&localRot));
	}

	void Transform::SetLocalRotationByRadians(const XMFLOAT3& radians)
	{
		XMVECTOR localRotation = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&radians));
		SetLocalRotationVector(localRotation);
	}

	void Transform::SetLocalRotationByDegrees(const XMFLOAT3& degrees)
	{
		m_localEulerDegrees = degrees;

		XMVECTOR q = XMQuaternionRotationRollPitchYaw(
			XMConvertToRadians(degrees.x),
			XMConvertToRadians(degrees.y),
			XMConvertToRadians(degrees.z)
		);
		
		XMStoreFloat4(&m_localRotation, q);

		m_isDirty = true;
	}

	void Transform::SetScaleVector(const XMVECTOR& worldScale)
	{
		auto safeWorldScale = XMVectorSelect(
			XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f),
			worldScale,
			XMVectorNotEqual(worldScale, XMVectorZero())
		);

		if (m_parent)
		{
			auto parentScale = m_parent->GetScaleVector();
			auto safeParentScale = XMVectorSelect(
				XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f),
				parentScale,
				XMVectorNotEqual(parentScale, XMVectorZero())
			);

			auto localScale = XMVectorDivide(safeWorldScale, safeParentScale);
			XMStoreFloat3(&m_localScale, localScale);
		}
		else
		{
			XMStoreFloat3(&m_localScale, safeWorldScale);
		}

		m_isDirty = true;
	}


	void Transform::SetScale(const XMFLOAT3& worldScale)
	{
		SetScaleVector(XMLoadFloat3(&worldScale));
	}

	void Transform::SetLocalScaleVector(const XMVECTOR& localScale)
	{
		auto safeLocalScale = XMVectorSelect(
			XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f),
			localScale,
			XMVectorNotEqual(localScale, XMVectorZero()));
		XMStoreFloat3(&m_localScale, safeLocalScale);

		m_isDirty = true;
	}

	void Transform::SetLocalScale(const XMFLOAT3& localScale)
	{
		SetLocalScaleVector(XMLoadFloat3(&localScale));
	}

	/*
	void Transform::Translate(const XMFLOAT3& move, eSpace space)
	{
		XMVECTOR moveVec = XMLoadFloat3(&move);

		if (space == eSpace::Scene)
		{
			// 월드 축 그대로 적용: 회전 금지
			XMVECTOR worldPos = GetPositionVector();
			XMVECTOR newWorldPos = XMVectorAdd(worldPos, moveVec);

			if (HasParent())
			{
				XMMATRIX parentInv = XMMatrixInverse(nullptr, m_parent->GetTransformMatrix());
				XMVECTOR localPos = XMVector3TransformCoord(newWorldPos, parentInv);
				SetLocalPositionVector(localPos);
			}
			else
			{
				SetLocalPositionVector(newWorldPos);
			}
		}
		else // Local
		{
			// 자기 회전 기준으로 이동 방향 회전
			XMVECTOR selfRot = GetRotationVector();
			XMVECTOR rotatedMove = XMVector3Rotate(moveVec, selfRot);

			// 로컬 위치에 더함
			XMVECTOR localPos = GetLocalPositionVector();
			SetLocalPositionVector(XMVectorAdd(localPos, rotatedMove));
		}
	}
	*/
	
	void Transform::Translate(const DirectX::XMFLOAT3& move, eSpace space)
	{
		auto moveVec = XMLoadFloat3(&move);

		if (space == eSpace::Scene)
		{
			// 월드 좌표계 기준으로 이동
			auto worldPos = GetPositionVector();
			auto newWorldPos = XMVectorAdd(worldPos, moveVec);

			if (HasParent())
			{
				// 부모 월드 행렬의 역행렬을 곱해 로컬 위치로 변환
				auto parentInv = XMMatrixInverse(nullptr, m_parent->GetTransformMatrix());
				auto localPos = XMVector3TransformCoord(newWorldPos, parentInv);
				SetLocalPositionVector(localPos);
			}
			else
			{
				SetLocalPositionVector(newWorldPos);
			}
		}
		else // Local
		{
			// 로컬 회전 기준으로 이동 방향 회전
			auto selfRot = GetRotationVector();//GetLocalRotationVector(); // 수정: 로컬 회전 사용
			auto rotatedMove = XMVector3Rotate(moveVec, selfRot);

			// 로컬 위치에 더함
			auto localPos = GetLocalPositionVector();
			auto newLocalPos = XMVectorAdd(localPos, rotatedMove);

			SetLocalPositionVector(newLocalPos);
		}
	}

	void Transform::Rotate(const XMFLOAT4& quaternion, eSpace space)
	{
		auto inputRot = XMLoadFloat4(&quaternion);

		if (space == eSpace::Scene)
		{
			// 월드 축 회전 누적: newWorld = input * currentWorld
			auto currentWorld = GetRotationVector();
			auto newWorld = XMQuaternionMultiply(inputRot, currentWorld);

			if (HasParent())
			{
				auto parentWorld = m_parent->GetRotationVector();
				auto parentInv = XMQuaternionInverse(parentWorld);
				auto newLocal = XMQuaternionMultiply(parentInv, newWorld);
				SetLocalRotationVector(newLocal);
			}
			else
			{
				SetLocalRotationVector(newWorld);
			}
		}
		else // Local
		{
			// 로컬 축 회전 누적: newLocal = input * currentLocal
			auto currentLocal = GetLocalRotationVector();
			auto newLocal = XMQuaternionMultiply(inputRot, currentLocal);
			SetLocalRotationVector(newLocal);
		}
	}

	void Transform::RotateByRadians(const XMFLOAT3& radians, eSpace space)
	{
		auto rotVec = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&radians));
		
		XMFLOAT4 quaternion;
		XMStoreFloat4(&quaternion, rotVec);
		Rotate(quaternion, space);
	}

	void Transform::RotateByDegrees(const XMFLOAT3& degrees, eSpace space)
	{
		RotateByRadians(
			{
				XMConvertToRadians(degrees.x),
				XMConvertToRadians(degrees.y),
				XMConvertToRadians(degrees.z)
			},
			space);
	}

	void Transform::LookAt(const XMFLOAT3& target, const DirectX::XMFLOAT3& worldUp)
	{
		XMVECTOR pos = XMLoadFloat3(&m_position);
		XMVECTOR tgt = XMLoadFloat3(&target);
		XMVECTOR up = XMLoadFloat3(&worldUp);

		XMVECTOR forward = XMVector3Normalize(XMVectorSubtract(tgt, pos));
		XMVECTOR right = XMVector3Normalize(XMVector3Cross(up, forward));
		XMVECTOR newUp = XMVector3Cross(forward, right);
		
		XMMATRIX lookAtMat = {
			XMVectorGetX(right),   XMVectorGetY(right),   XMVectorGetZ(right),   0.0f,
			XMVectorGetX(newUp),   XMVectorGetY(newUp),   XMVectorGetZ(newUp),   0.0f,
			XMVectorGetX(forward),   XMVectorGetY(forward),   XMVectorGetZ(forward),   0.0f,
			0.0f,                  0.0f,                  0.0f,                    1.0f
		};
		XMVECTOR quat = XMQuaternionRotationMatrix(lookAtMat);

		SetRotationVector(quat);
	}

	void Transform::SetTransformMatrix(const XMMATRIX& worldTransform)
	{
		XMVECTOR scale, rot, pos;
		if (XMMatrixDecompose(&scale, &rot, &pos, worldTransform))
		{
			if (m_parent)
			{
				auto parentWorld = m_parent->GetTransformMatrix();
				auto parentInv = XMMatrixInverse(nullptr, parentWorld);
				auto localMatrix = parentInv * worldTransform;

				XMMatrixDecompose(&scale, &rot, &pos, localMatrix);
			}

			XMStoreFloat3(&m_localScale, scale);
			XMStoreFloat4(&m_localRotation, XMQuaternionNormalize(rot));
			XMStoreFloat3(&m_localPosition, pos);

			m_localEulerDegrees = Math::QuaternionToDegrees(m_localRotation);
		}

		m_isDirty = true;
	}

	void Transform::SetTransform(const XMFLOAT4X4& worldTransform)
	{
		SetTransformMatrix(XMLoadFloat4x4(&worldTransform));
	}

	void Transform::SetLocalTransformMatrix(const XMMATRIX& localTransform)
	{
		XMVECTOR scale, rot, pos;
		if (XMMatrixDecompose(&scale, &rot, &pos, localTransform))
		{
			XMStoreFloat3(&m_localScale, scale);
			XMStoreFloat4(&m_localRotation, XMQuaternionNormalize(rot));
			XMStoreFloat3(&m_localPosition, pos);
		}

		m_isDirty = true;
	}

	void Transform::SetLocalTransform(const XMFLOAT4X4& localTransform)
	{
		SetLocalTransformMatrix(XMLoadFloat4x4(&localTransform));
	}

	XMVECTOR Transform::GetForwardVector() const
	{
		auto forward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		auto rot = GetRotationVector();
		forward = XMVector3Rotate(forward, rot);
		return XMVector3Normalize(forward);
	}

	XMFLOAT3 Transform::GetForward() const
	{
		XMFLOAT3 forward{};
		XMStoreFloat3(&forward, GetForwardVector());
		return forward;
	}

	XMVECTOR Transform::GetBackwardVector() const
	{
		auto backward = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
		auto rot = GetRotationVector();
		backward = XMVector3Rotate(backward, rot);
		return XMVector3Normalize(backward);
	}

	XMFLOAT3 Transform::GetBackward() const
	{
		XMFLOAT3 backward{};
		XMStoreFloat3(&backward, GetBackwardVector());
		return backward;
	}

	XMVECTOR Transform::GetRightVector() const
	{
		auto right = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		auto rot = GetRotationVector();
		right = XMVector3Rotate(right, rot);
		return XMVector3Normalize(right);
	}

	XMFLOAT3 Transform::GetRight() const
	{
		XMFLOAT3 right{};
		XMStoreFloat3(&right, GetRightVector());
		return right;
	}

	XMVECTOR Transform::GetLeftVector() const
	{
		auto left = XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f);
		auto rot = GetRotationVector();
		left = XMVector3Rotate(left, rot);
		return XMVector3Normalize(left);
	}

	XMFLOAT3 Transform::GetLeft() const
	{
		XMFLOAT3 left{};
		XMStoreFloat3(&left, GetLeftVector());
		return left;
	}

	XMVECTOR Transform::GetUpVector() const
	{
		auto up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		auto rot = GetRotationVector();
		up = XMVector3Rotate(up, rot);
		return XMVector3Normalize(up);
	}

	XMFLOAT3 Transform::GetUp() const
	{
		XMFLOAT3 up{};
		XMStoreFloat3(&up, GetUpVector());
		return up;
	}

	XMVECTOR Transform::GetDownVector() const
	{
		auto down = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
		auto rot = GetRotationVector();
		down = XMVector3Rotate(down, rot);
		return XMVector3Normalize(down);
	}

	XMFLOAT3 Transform::GetDown() const
	{
		XMFLOAT3 down{};
		XMStoreFloat3(&down, GetDownVector());
		return down;
	}

	XMVECTOR Transform::GetLocalForwardVector() const
	{
		auto forward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		auto rot = XMLoadFloat4(&m_localRotation);
		forward = XMVector3Rotate(forward, rot);
		return XMVector3Normalize(forward);
	}

	XMFLOAT3 Transform::GetLocalForward() const
	{
		XMFLOAT3 forward{};
		XMStoreFloat3(&forward, GetLocalForwardVector());
		return forward;
	}

	XMVECTOR Transform::GetLocalBackwardVector() const
	{
		auto backward = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
		auto rot = XMLoadFloat4(&m_localRotation);
		backward = XMVector3Rotate(backward, rot);
		return XMVector3Normalize(backward);
	}

	XMFLOAT3 Transform::GetLocalBackward() const
	{
		XMFLOAT3 backward{};
		XMStoreFloat3(&backward, GetLocalBackwardVector());
		return backward;
	}

	XMVECTOR Transform::GetLocalRightVector() const
	{
		auto right = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		auto rot = XMLoadFloat4(&m_localRotation);
		right = XMVector3Rotate(right, rot);
		return XMVector3Normalize(right);
	}

	XMFLOAT3 Transform::GetLocalRight() const
	{
		XMFLOAT3 right{};
		XMStoreFloat3(&right, GetLocalRightVector());
		return right;
	}

	XMVECTOR Transform::GetLocalLeftVector() const
	{
		auto left = XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f);
		auto rot = XMLoadFloat4(&m_localRotation);
		left = XMVector3Rotate(left, rot);
		return XMVector3Normalize(left);
	}

	XMFLOAT3 Transform::GetLocalLeft() const
	{
		XMFLOAT3 left{};
		XMStoreFloat3(&left, GetLocalLeftVector());
		return left;
	}

	XMVECTOR Transform::GetLocalUpVector() const
	{
		auto up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		auto rot = XMLoadFloat4(&m_localRotation);
		up = XMVector3Rotate(up, rot);
		return XMVector3Normalize(up);
	}

	XMFLOAT3 Transform::GetLocalUp() const
	{
		XMFLOAT3 up{};
		XMStoreFloat3(&up, GetLocalUpVector());
		return up;
	}

	XMVECTOR Transform::GetLocalDownVector() const
	{
		auto down = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
		auto rot = XMLoadFloat4(&m_localRotation);
		down = XMVector3Rotate(down, rot);
		return XMVector3Normalize(down);
	}

	XMFLOAT3 Transform::GetLocalDown() const
	{
		XMFLOAT3 down{};
		XMStoreFloat3(&down, GetLocalDownVector());
		return down;
	}

	void Transform::SetParent(Transform* parent)
	{
		if (parent == m_parent || IsParentOf(parent))
			return;

		DetachFromParent();

		if (parent)
		{
			parent->m_children.push_back(this);
			m_parent = parent;

			if (GetGameObject()->GetWorld())
				GetGameObject()->OnParentChanged();
		}
	}

	void Transform::DetachFromParent()
	{
		if (!m_parent)
			return;

		auto& siblings = m_parent->m_children;
		auto it = std::find(siblings.begin(), siblings.end(), this);
		if (it != siblings.end())
			siblings.erase(it);

		m_parent = nullptr;
		m_isDirty = true;

		if (GetGameObject()->GetWorld())
			GetGameObject()->OnParentChanged();
	}

	bool Transform::IsParentOf(Transform* target)
	{
		if (!target || m_children.empty())
			return false;

		if (target == this)
			return false;

		if (target->m_parent == this)
			return true;

		for (auto child : m_children)
		{
			if (child->IsParentOf(target))
				return true;
		}

		return false;
	}

	bool Transform::IsChildOf(Transform* parent)
	{
		if(!parent ||  m_parent == nullptr)
			return false;

		if (m_parent == parent)
			return true;
		
		return m_parent->IsChildOf(parent);
	}

	Transform* Transform::GetChild(size_t index)
	{
		return m_children.size() > index ? m_children[index] : nullptr;
	}

	Transform* Transform::Find(const std::string& name)
	{
		for (auto& child : m_children)
		{
			if (child->GetGameObject()->GetName() == name)
				return child;
		}

		return nullptr;
	}

	size_t Transform::GetChildCount()
	{
		return m_children.size();
	}

	void Transform::DetachChildren()
	{
		for (auto& child : m_children)
			child->SetParent(nullptr);
		
		m_children.clear();
	}

	void Transform::RemoveChild(Transform* child)
	{
		if (!child)
			return;

		auto it = std::find(m_children.begin(), m_children.end(), child);
		if (it == m_children.end())
			return;

		m_children.erase(it);
	}

	size_t Transform::GetSiblingIndex()
	{
		assert(GetGameObject());

		if (HasParent())
		{
			const auto& sibling = m_parent->GetChildren();
			auto it = std::find(sibling.begin(), sibling.end(), this);
			
			return (it != sibling.end()) ? std::distance(sibling.begin(), it) : std::numeric_limits<size_t>::max();
		}
		else
		{
			const auto& roots = GetGameObject()->GetWorld()->m_rootGameObjects;
			auto it = std::find(roots.begin(), roots.end(), GetGameObject());

			return (it != roots.end()) ? std::distance(roots.begin(), it) : std::numeric_limits<size_t>::max();
		}
	}

	void Transform::SetSiblingIndex(size_t index)
	{
		assert(GetGameObject());

		if (HasParent())
		{
			auto& sibling = m_parent->GetChildren();
			if (index >= sibling.size())
				return;

			auto it = std::find(sibling.begin(), sibling.end(), this);
			if (it != sibling.end())
			{
				sibling.erase(it);
				sibling.insert(sibling.begin() + index, this);
			}
		}
		else
		{
			auto& roots = GetGameObject()->GetWorld()->m_rootGameObjects;
			if (index >= roots.size())
				return;

			auto it = std::find(roots.begin(), roots.end(), GetGameObject());
			if (it != roots.end())
			{
				roots.erase(it);
				roots.insert(roots.begin() + index, GetGameObject());
			}
		}
	}

	bool Transform::IsOverridden()
	{
		// 프리팹 제거 과정에서 구현 제거
		return false;
	}
}
