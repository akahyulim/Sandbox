#include "pch.h"
#include "Transform.h"
#include "Utilities/Math.h"
#include "Scene/GameObject.h" 

using namespace DirectX;

namespace Dive
{
	Transform::Transform(GameObject* owner)
		: Component(owner)
	{
	}

	Transform::~Transform()
	{
	}

	void Transform::Update()
	{
		if (!m_isDirty)
			return;

		auto localMatrix = GetLocalMatrix();

		if (m_parent)
		{
			m_parent->Update();
			m_worldMatrix = localMatrix * m_parent->m_worldMatrix;
		}
		else
		{
			m_worldMatrix = localMatrix;
		}

		m_isDirty = false;

		// 루트부터 시작해 하향식으로 자식들을 갱신 필요상태로 변경
		for (auto child : m_children)
			child->m_isDirty = true;
	}

	DirectX::XMVECTOR Transform::GetPositionVector()
	{
		if (m_isDirty)
			Update();

		return m_worldMatrix.r[3];
	}

	DirectX::XMFLOAT3 Transform::GetPosition()
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMStoreFloat3(&pos, GetPositionVector());
		return pos;
	}

	void Transform::SetPositionVector(const XMVECTOR& worldPos)
	{
		if (m_parent)
		{
			auto parentInv = XMMatrixInverse(nullptr, m_parent->GetWorldMatrix());
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

	void Transform::SetPosition(float x, float y, float z)
	{
		SetPosition({ x, y, z });
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

	void Transform::SetLocalPosition(float x, float y, float z)
	{
		m_localPosition = { x, y, z };
		m_isDirty = true;
	}

	DirectX::XMVECTOR Transform::GetRotationVector()
	{
		if (m_isDirty)
			Update();

		DirectX::XMVECTOR s, r, t;
		DirectX::XMMatrixDecompose(&s, &r, &t, m_worldMatrix);

		return r;
	}

	DirectX::XMFLOAT4 Transform::GetRotation()
	{
		DirectX::XMFLOAT4 rot;
		DirectX::XMStoreFloat4(&rot, GetRotationVector());
		return rot;
	}

	XMFLOAT3 Transform::GetRotationRadians()
	{
		auto rotMatrix = XMMatrixRotationQuaternion(GetRotationVector());

		float pitch = std::atan2(rotMatrix.r[1].m128_f32[2], rotMatrix.r[2].m128_f32[2]);
		float yaw = std::atan2(-rotMatrix.r[0].m128_f32[2], std::sqrt(rotMatrix.r[0].m128_f32[0] * rotMatrix.r[0].m128_f32[0] + rotMatrix.r[0].m128_f32[1] * rotMatrix.r[0].m128_f32[1]));
		float roll = std::atan2(rotMatrix.r[0].m128_f32[1], rotMatrix.r[0].m128_f32[0]);

		return { pitch, yaw, roll };
	}

	XMFLOAT3 Transform::GetRotationDegrees()
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

	DirectX::XMVECTOR Transform::GetScaleVector()
	{
		if (m_isDirty)
			Update();

		DirectX::XMVECTOR s, r, t;
		DirectX::XMMatrixDecompose(&s, &r, &t, m_worldMatrix);

		return s;
	}

	DirectX::XMFLOAT3 Transform::GetScale()
	{
		DirectX::XMFLOAT3 scale;
		DirectX::XMStoreFloat3(&scale, GetScaleVector());
		return scale;
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

	void Transform::TranslateVector(DirectX::FXMVECTOR translation, eSpace space)
	{
		DirectX::XMVECTOR currentLocalPos = DirectX::XMLoadFloat3(&m_localPosition);

		if (space == eSpace::Local)
		{
			DirectX::XMVECTOR rotatedTranslation = DirectX::XMVector3Rotate(translation, DirectX::XMLoadFloat4(&m_localRotation));
			currentLocalPos = DirectX::XMVectorAdd(currentLocalPos, rotatedTranslation);
		}
		else
		{
			if (m_parent)
			{
				DirectX::XMMATRIX parentWorldMat = m_parent->GetWorldMatrix();
				DirectX::XMMATRIX parentInv = DirectX::XMMatrixInverse(nullptr, parentWorldMat);

				DirectX::XMVECTOR localTranslation = DirectX::XMVector3TransformCoord(translation, parentInv) -
					DirectX::XMVector3TransformCoord(DirectX::XMVectorZero(), parentInv);

				currentLocalPos = DirectX::XMVectorAdd(currentLocalPos, localTranslation);
			}
			else
			{
				currentLocalPos = DirectX::XMVectorAdd(currentLocalPos, translation);
			}
		}

		DirectX::XMStoreFloat3(&m_localPosition, currentLocalPos);

		SetDirty();
	}

	void Transform::Translate(const DirectX::XMFLOAT3& translation, eSpace space)
	{
		DirectX::XMVECTOR t = DirectX::XMLoadFloat3(&translation);
		TranslateVector(t, space);
	}

	void Transform::RotateVector(DirectX::FXMVECTOR quaternion, eSpace space)
	{
		if (space == eSpace::Local)
		{
			DirectX::XMVECTOR currentLocalRot = DirectX::XMLoadFloat4(&m_localRotation);
			
			DirectX::XMVECTOR newLocalRot = DirectX::XMQuaternionMultiply(currentLocalRot, quaternion);
			DirectX::XMStoreFloat4(&m_localRotation, newLocalRot);

			SetDirty();
		}
		else 
		{
			DirectX::XMVECTOR currentWorldRot = GetRotationVector();
			DirectX::XMVECTOR newWorldRot = DirectX::XMQuaternionMultiply(quaternion, currentWorldRot);

			SetRotationVector(newWorldRot);
		}
	}

	void Transform::Rotate(const DirectX::XMFLOAT4& quaternion, eSpace space)
	{
		DirectX::XMVECTOR q = DirectX::XMLoadFloat4(&quaternion);
		RotateVector(q, space);
	}

	void Transform::RotateByRadians(const XMFLOAT3& radians, eSpace space)
	{
		auto rotQuat = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&radians));
		RotateVector(rotQuat, space);
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
		XMVECTOR pos = GetPositionVector();
		XMVECTOR tgt = XMLoadFloat3(&target);
		XMVECTOR up = XMLoadFloat3(&worldUp);

		XMVECTOR forward = XMVector3Normalize(XMVectorSubtract(tgt, pos));
		if (XMVector3Equal(forward, XMVectorZero())) 
			return;
		forward = XMVector3Normalize(forward);
		float dot = XMVectorGetX(XMVector3Dot(forward, up));
		if (fabsf(dot) > 0.999f)
			up = (fabsf(XMVectorGetY(forward)) > 0.999f) ? XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f) : XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		XMVECTOR right = XMVector3Normalize(XMVector3Cross(up, forward));
		XMVECTOR newUp = XMVector3Cross(forward, right);

		XMMATRIX lookAtMat = {
			XMVectorGetX(right), XMVectorGetY(right), XMVectorGetZ(right), 0.0f,
			XMVectorGetX(newUp), XMVectorGetY(newUp), XMVectorGetZ(newUp), 0.0f,
			XMVectorGetX(forward), XMVectorGetY(forward), XMVectorGetZ(forward), 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
		XMVECTOR quat = XMQuaternionRotationMatrix(lookAtMat);

		SetRotationVector(quat);
	}

	DirectX::XMMATRIX Transform::GetWorldMatrix()
	{
		if (m_isDirty)
			Update();

		return m_worldMatrix;
	}

	void Transform::SetWorldMatrix(const DirectX::XMMATRIX& worldMat)
	{
		DirectX::XMMATRIX localMat;

		if (m_parent)
		{
			DirectX::XMMATRIX parentWorldMat = m_parent->GetWorldMatrix();
			DirectX::XMMATRIX parentInv = DirectX::XMMatrixInverse(nullptr, parentWorldMat);

			localMat = worldMat * parentInv;
		}
		else
		{
			localMat = worldMat;
		}

		SetLocalMatrix(localMat);
	}

	DirectX::XMMATRIX Transform::GetLocalMatrix() const
	{
		return DirectX::XMMatrixScalingFromVector(XMLoadFloat3(&m_localScale)) *
			DirectX::XMMatrixRotationQuaternion(XMLoadFloat4(&m_localRotation)) *
			DirectX::XMMatrixTranslationFromVector(XMLoadFloat3(&m_localPosition));
	}

	void Transform::SetLocalMatrix(const DirectX::XMMATRIX& localMat)
	{
		DirectX::XMVECTOR s, r, t;
		DirectX::XMMatrixDecompose(&s, &r, &t, localMat);

		DirectX::XMStoreFloat3(&m_localPosition, t);
		DirectX::XMStoreFloat4(&m_localRotation, r);
		DirectX::XMStoreFloat3(&m_localScale, s);

		m_isDirty = true;
	}

	XMVECTOR Transform::GetForwardVector()
	{
		if (m_isDirty)
			Update();

		return DirectX::XMVector3Normalize(m_worldMatrix.r[2]);
	}

	XMFLOAT3 Transform::GetForward()
	{
		XMFLOAT3 forward{};
		XMStoreFloat3(&forward, GetForwardVector());
		return forward;
	}

	XMVECTOR Transform::GetRightVector()
	{
		if (m_isDirty)
			Update();

		return DirectX::XMVector3Normalize(m_worldMatrix.r[0]);
	}

	XMFLOAT3 Transform::GetRight()
	{
		XMFLOAT3 right{};
		XMStoreFloat3(&right, GetRightVector());
		return right;
	}

	XMVECTOR Transform::GetUpVector()
	{
		if (m_isDirty)
			Update();

		return DirectX::XMVector3Normalize(m_worldMatrix.r[1]);
	}

	XMFLOAT3 Transform::GetUp()
	{
		XMFLOAT3 up{};
		XMStoreFloat3(&up, GetUpVector());
		return up;
	}

	XMVECTOR Transform::GetLocalForwardVector() const
	{
		XMVECTOR baseForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		return DirectX::XMVector3Rotate(baseForward, XMLoadFloat4(&m_localRotation));
	}

	XMFLOAT3 Transform::GetLocalForward() const
	{
		XMFLOAT3 forward{};
		XMStoreFloat3(&forward, GetLocalForwardVector());
		return forward;
	}

	XMVECTOR Transform::GetLocalRightVector() const
	{
		XMVECTOR baseRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		return DirectX::XMVector3Rotate(baseRight, XMLoadFloat4(&m_localRotation));
	}

	XMFLOAT3 Transform::GetLocalRight() const
	{
		XMFLOAT3 right{};
		XMStoreFloat3(&right, GetLocalRightVector());
		return right;
	}

	XMVECTOR Transform::GetLocalUpVector() const
	{
		XMVECTOR baseUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		return DirectX::XMVector3Rotate(baseUp, XMLoadFloat4(&m_localRotation));
	}

	XMFLOAT3 Transform::GetLocalUp() const
	{
		XMFLOAT3 up{};
		XMStoreFloat3(&up, GetLocalUpVector());
		return up;
	}

	void Transform::SetDirty()
	{
		if (m_isDirty) 
			return;

		m_isDirty = true;

		for (auto child : m_children)
		{
			child->SetDirty();
		}
	}
}
