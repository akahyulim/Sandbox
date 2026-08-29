#pragma once
#include "Component.h"

#include <DirectXMath.h>

namespace Dive
{
	class GameObject;
	class Texture;

	enum class eProjectionType
	{
		Perspective,
		Orthographic
	};

	class Camera : public Component
	{
	public:
		Camera(GameObject* owner);
		virtual ~Camera() override = default;

		DirectX::XMFLOAT4X4 GetView() const;
		DirectX::XMMATRIX GetViewMatrix() const;
		DirectX::XMFLOAT4X4 GetProjection() const;
		DirectX::XMMATRIX GetProjectionMatrix() const;
		DirectX::XMFLOAT4X4 GetViewProj() const;
		DirectX::XMMATRIX GetViewProjMatrix() const;

		eProjectionType GetProjectionType() const { return m_projectionType; }
		void SetProjectionType(eProjectionType type) { m_projectionType = type; }

		void SetAspectRatio(float width, float height);

		float GetFieldOfView() const { return m_fov; }
		void SetFieldOfView(float fov);

		float GetNearClipPlane() const { return m_nearClip; }
		void SetNearClipPlane(float nearPlane);

		float GetFarClipPlane() const { return m_farClip; }
		void SetFarClipPlane(float farPlane);

		// 사용하는 곳이 있나...?
		static constexpr eComponentType GetType() { return eComponentType::Camera; }

	private:
		eProjectionType m_projectionType = eProjectionType::Perspective;

		float m_fov = 45.0f;
		float m_nearClip = 0.1f;
		float m_farClip = 5000.0f;

		float m_aspectRatio = 0.0f;
	};
}