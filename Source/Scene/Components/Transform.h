#pragma once
#include "Component.h"

#include <DirectXMath.h>

namespace Dive
{
	class GameObject;

	enum class eSpace
	{
		World,
		Local
	};

	class alignas(16) Transform : public Component
	{
	public:
		Transform(GameObject* owner);
		~Transform() override;

		static constexpr eComponentType GetComponentType() { return eComponentType::Transform; }

		virtual void Update() override;

		// transform
		DirectX::XMVECTOR GetPositionVector();
		DirectX::XMFLOAT3 GetPosition();
		void SetPositionVector(const DirectX::XMVECTOR& worldPos);
		void SetPosition(const DirectX::XMFLOAT3& worldPos);
		void SetPosition(float x, float y, float z);
		DirectX::XMVECTOR GetLocalPositionVector() const { return DirectX::XMLoadFloat3(&m_localPosition); }
		const DirectX::XMFLOAT3& GetLocalPosition() const { return m_localPosition; }
		void SetLocalPositionVector(const DirectX::XMVECTOR& localPos);
		void SetLocalPosition(const DirectX::XMFLOAT3& localPos);
		void SetLocalPosition(float x, float y, float z);

		DirectX::XMVECTOR GetRotationVector();
		DirectX::XMFLOAT4 GetRotation();
		DirectX::XMFLOAT3 GetRotationRadians();
		DirectX::XMFLOAT3 GetRotationDegrees();
		void SetRotationVector(const DirectX::XMVECTOR& worldRot);
		void SetRotation(const DirectX::XMFLOAT4& worldRot);
		void SetRotationByRadians(const DirectX::XMFLOAT3& radians);
		void SetRotationByDegrees(const DirectX::XMFLOAT3& degrees);
		DirectX::XMVECTOR GetLocalRotationVector() const { return DirectX::XMLoadFloat4(&m_localRotation); }
		const DirectX::XMFLOAT4& GetLocalRotation() const { return m_localRotation; }
		DirectX::XMFLOAT3 GetLocalRotationRadians() const;
		const DirectX::XMFLOAT3& GetLocalRotationDegrees() const { return m_localEulerDegrees; }
		void SetLocalRotationVector(const DirectX::XMVECTOR& localRot);
		void SetLocalRotation(const DirectX::XMFLOAT4& localRot);
		void SetLocalRotationByRadians(const DirectX::XMFLOAT3& radians);
		void SetLocalRotationByDegrees(const DirectX::XMFLOAT3& degrees);

		DirectX::XMVECTOR GetScaleVector();
		DirectX::XMFLOAT3 GetScale();
		void SetScaleVector(const DirectX::XMVECTOR& worldScale);
		void SetScale(const DirectX::XMFLOAT3& worldScale);
		DirectX::XMVECTOR GetLocalScaleVector() const { return DirectX::XMLoadFloat3(&m_localScale); }
		const DirectX::XMFLOAT3& GetLocalScale() const { return m_localScale; }
		void SetLocalScaleVector(const DirectX::XMVECTOR& localScale);
		void SetLocalScale(const DirectX::XMFLOAT3& localScale);

		void TranslateVector(DirectX::FXMVECTOR translation, eSpace space = eSpace::Local);
		void Translate(const DirectX::XMFLOAT3& translation, eSpace space = eSpace::Local);

		void RotateVector(DirectX::FXMVECTOR quaternion, eSpace space = eSpace::Local);
		void Rotate(const DirectX::XMFLOAT4& quaternion, eSpace space = eSpace::Local);
		void RotateByRadians(const DirectX::XMFLOAT3& radians, eSpace space = eSpace::Local);
		void RotateByDegrees(const DirectX::XMFLOAT3& degrees, eSpace space = eSpace::Local);

		void LookAt(const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& worldUp = { 0.0f, 1.0f, 0.0f });

		DirectX::XMMATRIX GetWorldMatrix();
		void SetWorldMatrix(const DirectX::XMMATRIX& worldMat);

		DirectX::XMMATRIX GetLocalMatrix() const;
		void SetLocalMatrix(const DirectX::XMMATRIX& localMat);

		DirectX::XMVECTOR GetForwardVector();
		DirectX::XMFLOAT3 GetForward();
		DirectX::XMVECTOR GetRightVector();
		DirectX::XMFLOAT3 GetRight();
		DirectX::XMVECTOR GetUpVector();
		DirectX::XMFLOAT3 GetUp();

		DirectX::XMVECTOR GetLocalForwardVector() const;
		DirectX::XMFLOAT3 GetLocalForward() const;
		DirectX::XMVECTOR GetLocalRightVector() const;
		DirectX::XMFLOAT3 GetLocalRight() const;
		DirectX::XMVECTOR GetLocalUpVector() const;
		DirectX::XMFLOAT3 GetLocalUp() const;

		void SetDirty();

	private:
		DirectX::XMFLOAT3 m_localPosition = { 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT4 m_localRotation = { 0.0f, 0.0f, 0.0f, 1.0f };
		DirectX::XMFLOAT3 m_localScale = { 1.0f, 1.0f, 1.0f };

		DirectX::XMFLOAT3 m_localEulerDegrees = { 0.0f, 0.0f, 0.0f };

		DirectX::XMMATRIX m_worldMatrix = DirectX::XMMatrixIdentity();

		Transform* m_parent = nullptr;
		std::vector<Transform*> m_children;
	};
}