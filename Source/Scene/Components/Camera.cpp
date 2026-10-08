#include "pch.h"
#include "Camera.h"
#include "Transform.h"

using namespace DirectX;

namespace Dive
{
	namespace
	{
		static constexpr float FOV_MIN = 1.0f;
		static constexpr float FOV_MAX = 160.0f;

		static constexpr float NEAR_CLIP_PLANE = 0.1f;
	}

	Camera::Camera(GameObject* owner)
		: Component(owner)
	{
		XMStoreFloat4x4(&m_projection, XMMatrixIdentity());
	}

	DirectX::XMFLOAT4X4 Camera::GetView() const
	{
		XMFLOAT4X4 view{};
		XMStoreFloat4x4(&view, GetViewMatrix());
		return view;
	}

	DirectX::XMMATRIX Camera::GetViewMatrix() const
	{
		auto* transform = GetTransform();
		assert(transform);

		return XMMatrixLookToLH(
			transform->GetPositionVector(), 
			transform->GetForwardVector(), 
			transform->GetUpVector()
		);
	}

	DirectX::XMFLOAT4X4 Camera::GetProjection() const
	{
		return m_projection;
	}

	DirectX::XMMATRIX Camera::GetProjectionMatrix() const
	{
		return XMLoadFloat4x4(&m_projection);
	}

	DirectX::XMFLOAT4X4 Camera::GetViewProj() const
	{
		XMFLOAT4X4 viewProj;
		XMStoreFloat4x4(&viewProj, GetViewProjMatrix());

		return viewProj;
	}

	DirectX::XMMATRIX Camera::GetViewProjMatrix() const
	{
		return DirectX::XMMatrixMultiply(GetViewMatrix(), GetProjectionMatrix());
	}

	DirectX::XMFLOAT4X4 Camera::GetInverseViewProj() const
	{
		XMFLOAT4X4 invViewProj;
		XMStoreFloat4x4(&invViewProj, GetInverseViewProjMatrix());
		return invViewProj;
	}

	DirectX::XMMATRIX Camera::GetInverseViewProjMatrix() const
	{
		return XMMatrixInverse(nullptr, GetViewProjMatrix());
	}

	void Camera::SetProjectionType(eProjectionType type)
	{
		if (m_projectionType == type)
			return;

		m_projectionType = type;
		updateProjection();
	}

	void Camera::SetAspectRatio(float width, float height)
	{
		float value = width / height;

		if (fabsf(m_aspectRatio - value) < 1e-5f)
			return;

		m_aspectRatio = value;
		updateProjection();
	}

	void Camera::SetFieldOfView(float fov)
	{
		if (m_fov == fov)
			return;

		if (fov < FOV_MIN)
			m_fov = FOV_MIN;
		else if (fov > FOV_MAX)
			m_fov = FOV_MAX;
		else
			m_fov = fov;

		updateProjection();
	}

	void Camera::SetNearClipPlane(float nearPlane)
	{
		if (nearPlane < NEAR_CLIP_PLANE)
		{
			spdlog::warn("[::SetNearClipPlane] 유효하지 않은 Near Clip 값: {}", nearPlane);
			return;
		}

		if (fabsf(m_nearClip - nearPlane) < 1e-5f)
			return;

		m_nearClip = nearPlane; 
		updateProjection();
	}

	void Camera::SetFarClipPlane(float farPlane)
	{
		if (farPlane <= m_nearClip)
		{
			spdlog::warn("유효하지 않은 Far Clip 값 전달: {}", farPlane);
			return;
		}

		if (fabsf(m_farClip - farPlane) < 1e-5f)
			return;

		m_farClip = farPlane;
		updateProjection();
	}

	void Camera::SetOrthoHeight(float height)
	{
		if (fabsf(m_orthoHeight - height) < 1e-5f)
			return;

		m_orthoHeight = height;
		updateProjection();
	}

	DirectX::XMFLOAT4 Camera::GetPosition() const
	{
		auto transform = GetTransform();
		assert(transform);

		auto pos = transform->GetPosition();
		return DirectX::XMFLOAT4(pos.x, pos.y, pos.z, 1.0f);
	}

	DirectX::XMFLOAT4 Camera::GetForward() const
	{
		auto transform = GetTransform();
		assert(transform);

		auto forward = transform->GetForward();
		return DirectX::XMFLOAT4(forward.x, forward.y, forward.z, 0.0f);
	}

	void Camera::updateProjection()
	{
		if (m_projectionType == eProjectionType::Perspective)
		{
			XMStoreFloat4x4(&m_projection, 
				XMMatrixPerspectiveFovLH(
					XMConvertToRadians(m_fov),
					m_aspectRatio,
					m_nearClip,
					m_farClip
				));
		}
		else
		{
			float viewWidth = m_orthoHeight * m_aspectRatio;

			XMStoreFloat4x4(&m_projection,
				XMMatrixOrthographicLH(
					viewWidth,
					m_orthoHeight,
					m_nearClip,
					m_farClip
				));
		}
	}
}