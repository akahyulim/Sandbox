#pragma once
#include "Component.h"

#include <DirectXMath.h>

namespace Dive
{
	class GameObject;

	enum class eSpace
	{
		Scene,
		Local
	};

	class Transform : public Component
	{
	public:
		Transform(GameObject* owner);
		~Transform() override;

		static constexpr eComponentType GetComponentType() { return eComponentType::Transform; }

		virtual void Update() override;

		// transform
		DirectX::XMVECTOR GetPositionVector() const { return DirectX::XMLoadFloat3(&m_position); }
		const DirectX::XMFLOAT3& GetPosition() const { return m_position; }
		void SetPositionVector(const DirectX::XMVECTOR& worldPos);
		void SetPosition(const DirectX::XMFLOAT3& worldPos);
		void SetPosition(float x, float y, float z);
		DirectX::XMVECTOR GetLocalPositionVector() const { return DirectX::XMLoadFloat3(&m_localPosition); }
		const DirectX::XMFLOAT3& GetLocalPosition() const { return m_localPosition; }
		void SetLocalPositionVector(const DirectX::XMVECTOR& localPos);
		void SetLocalPosition(const DirectX::XMFLOAT3& localPos);
		void SetLocalPosition(float x, float y, float z);

		DirectX::XMVECTOR GetRotationVector() const { return DirectX::XMLoadFloat4(&m_rotation); }
		const DirectX::XMFLOAT4& GetRotation() const { return m_rotation; }
		DirectX::XMFLOAT3 GetRotationRadians() const;
		DirectX::XMFLOAT3 GetRotationDegrees() const;
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

		DirectX::XMVECTOR GetScaleVector() const { return DirectX::XMLoadFloat3(&m_scale); }
		const DirectX::XMFLOAT3& GetScale() const { return m_scale; }
		void SetScaleVector(const DirectX::XMVECTOR& worldScale);
		void SetScale(const DirectX::XMFLOAT3& worldScale);
		DirectX::XMVECTOR GetLocalScaleVector() const { return DirectX::XMLoadFloat3(&m_localScale); }
		const DirectX::XMFLOAT3& GetLocalScale() const { return m_localScale; }
		void SetLocalScaleVector(const DirectX::XMVECTOR& localScale);
		void SetLocalScale(const DirectX::XMFLOAT3& localScale);

		void Translate(const DirectX::XMFLOAT3& move, eSpace space = eSpace::Local);
		void Rotate(const DirectX::XMFLOAT4& quaternion, eSpace space = eSpace::Local);
		void RotateByRadians(const DirectX::XMFLOAT3& radians, eSpace space = eSpace::Local);
		void RotateByDegrees(const DirectX::XMFLOAT3& degrees, eSpace space = eSpace::Local);

		void LookAt(const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& worldUp = { 0.0f, 1.0f, 0.0f });

		DirectX::XMMATRIX GetTransformMatrix() const { return XMLoadFloat4x4(&m_transform); }
		const DirectX::XMFLOAT4X4& GetTransform() const { return m_transform; }
		void SetTransformMatrix(const DirectX::XMMATRIX& worldTranform);
		void SetTransform(const DirectX::XMFLOAT4X4& worldTransform);
		DirectX::XMMATRIX GetLocalTransformMatrix() const { return XMLoadFloat4x4(&m_localTransform); }
		const DirectX::XMFLOAT4X4& GetLocalTransform() const { return m_localTransform; }
		void SetLocalTransformMatrix(const DirectX::XMMATRIX& localTranform);
		void SetLocalTransform(const DirectX::XMFLOAT4X4& localTransform);

		DirectX::XMVECTOR GetForwardVector() const;
		DirectX::XMFLOAT3 GetForward() const;
		DirectX::XMVECTOR GetBackwardVector() const;
		DirectX::XMFLOAT3 GetBackward() const;
		DirectX::XMVECTOR GetRightVector() const;
		DirectX::XMFLOAT3 GetRight() const;
		DirectX::XMVECTOR GetLeftVector() const;
		DirectX::XMFLOAT3 GetLeft() const;
		DirectX::XMVECTOR GetUpVector() const;
		DirectX::XMFLOAT3 GetUp() const;
		DirectX::XMVECTOR GetDownVector() const;
		DirectX::XMFLOAT3 GetDown() const;

		DirectX::XMVECTOR GetLocalForwardVector() const;
		DirectX::XMFLOAT3 GetLocalForward() const;
		DirectX::XMVECTOR GetLocalBackwardVector() const;
		DirectX::XMFLOAT3 GetLocalBackward() const;
		DirectX::XMVECTOR GetLocalRightVector() const;
		DirectX::XMFLOAT3 GetLocalRight() const;
		DirectX::XMVECTOR GetLocalLeftVector() const;
		DirectX::XMFLOAT3 GetLocalLeft() const;
		DirectX::XMVECTOR GetLocalUpVector() const;
		DirectX::XMFLOAT3 GetLocalUp() const;
		DirectX::XMVECTOR GetLocalDownVector() const;
		DirectX::XMFLOAT3 GetLocalDown() const;

		// hierarchy
		bool HasParent() const { return m_parent != nullptr; }
		Transform* GetParent() const { return m_parent; }
		void SetParent(Transform* parent);
		void DetachFromParent();
		bool IsParentOf(Transform* target);

		bool HasChildren() const { return !m_children.empty(); }
		std::vector<Transform*>& GetChildren() { return m_children; }

		bool IsChildOf(Transform* parent);

		Transform* GetChild(size_t index);
		Transform* Find(const std::string& name);

		size_t GetChildCount();

		void DetachChildren();
		void RemoveChild(Transform* child);

		size_t GetSiblingIndex();
		void SetSiblingIndex(size_t index);

	private:
		DirectX::XMFLOAT3 m_localPosition{ 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT4 m_localRotation{ 0.0f, 0.0f, 0.0f, 1.0f };
		DirectX::XMFLOAT3 m_localScale{ 1.0f, 1.0f, 1.0f };

		DirectX::XMFLOAT3 m_localEulerDegrees{ 0.0f, 0.0f, 0.0f };

		DirectX::XMFLOAT3 m_position{ 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT4 m_rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
		DirectX::XMFLOAT3 m_scale{ 1.0f, 1.0f, 1.0f };

		DirectX::XMFLOAT4X4 m_localTransform;
		DirectX::XMFLOAT4X4 m_transform;

		Transform* m_parent = nullptr;
		std::vector<Transform*> m_children;
	};
}