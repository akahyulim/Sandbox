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

	Camera::Camera(GameObject* obj)
		: Component(obj)
	{
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
		XMFLOAT4X4 projection{};
		XMStoreFloat4x4(&projection, GetProjectionMatrix());
		return projection;
	}

	DirectX::XMMATRIX Camera::GetProjectionMatrix() const
	{
		if (m_projectionType == eProjectionType::Perspective)
		{
			return XMMatrixPerspectiveFovLH(
				XMConvertToRadians(m_fov),
				GetAspectRatio(),
				m_nearClip,
				m_farClip
			);
		}
		else
		{
			// 코파일럿이 작성한  코드다.
			// width, height가 맞는지 모르겠다.
			return XMMatrixOrthographicLH(
				GetAspectRatio() * m_farClip,
				m_farClip,
				m_nearClip,
				m_farClip
			);
		}
	}

	DirectX::XMFLOAT4X4 Camera::GetViewProj() const
	{
		XMFLOAT4X4 viewProj;
		XMStoreFloat4x4(&viewProj, GetViewProjMatrix());

		return viewProj;
	}

	DirectX::XMMATRIX Camera::GetViewProjMatrix() const
	{
		return GetViewMatrix() * GetProjectionMatrix();
	}

	float Camera::GetAspectRatio() const
	{
		return (m_viewport.width / m_viewport.height);
	}

	void Camera::SetFieldOfView(float fov)
	{
		if (fov < FOV_MIN)
			m_fov = FOV_MIN;
		else if (fov > FOV_MAX)
			m_fov = FOV_MAX;
		else
			m_fov = fov;
	}

	void Camera::SetNearClipPlane(float nearPlane)
	{
		if (nearPlane < NEAR_CLIP_PLANE)
		{
			spdlog::warn("[::SetNearClipPlane] 유효하지 않은 Near Clip 값: {}", nearPlane);
			return;
		}
		m_nearClip = nearPlane;
	}

	void Camera::SetFarClipPlane(float farPlane)
	{
		if (farPlane <= m_nearClip)
		{
			spdlog::warn("유효하지 않은 Far Clip 값 전달: {}", farPlane);
			return;
		}
		m_farClip = farPlane;
	}

	void Camera::SetViewport(float topLeftX, float topLeftY, float width, float height, float minDepth, float maxDepth)
	{
		m_viewport.topLeftX = topLeftX;
		m_viewport.topLeftY = topLeftY;
		m_viewport.width = width;
		m_viewport.height = height;
		m_viewport.minDepth = minDepth;
		m_viewport.maxDepth = maxDepth;
	}
}