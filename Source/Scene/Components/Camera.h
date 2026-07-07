#pragma once
#include "Component.h"

#include <DirectXMath.h>

namespace Dive
{
	class GameObject;
	class RenderTexture;

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

		float GetAspectRatio() const;

		float GetFieldOfView() const { return m_fov; }
		void SetFieldOfView(float fov);

		float GetNearClipPlane() const { return m_nearClip; }
		void SetNearClipPlane(float nearPlane);

		float GetFarClipPlane() const { return m_farClip; }
		void SetFarClipPlane(float farPlane);

		// 이걸로 설정하면 윈도우 크기 변화가 적용되지 않는다.
		// Update 등에서 확인 후 갱신이 필요하다.
		// 그리고 카메라가 두 개 이상 혹은 화면 분할을 염두하여 기능을 추가해야한다.
		void SetViewport(float topLeftX, float topLeftY, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f);
		Viewport GetViewport() const { return m_viewport; }

		std::shared_ptr<RenderTexture> GetTargetTexture() const { return m_targetTexture; }
		void SetTargetTexture(std::shared_ptr<RenderTexture> rt) { m_targetTexture = rt; }

		Color GetClearColor() const { return m_clearColor; }
		void SetClearColor(const Color& color) { m_clearColor = color; }
		void SetClearColor(float r, float g, float b) { m_clearColor = Color{ r, g, b, 1.0f }; }

		// 사용하는 곳이 있나...?
		static constexpr eComponentType GetType() { return eComponentType::Camera; }

	private:
		eProjectionType m_projectionType = eProjectionType::Perspective;

		float m_fov = 45.0f;
		float m_nearClip = 0.1f;
		float m_farClip = 5000.0f;

		Viewport m_viewport;
		std::shared_ptr<RenderTexture> m_targetTexture;

		Color m_clearColor = Color::DeepSkyBlue;
	};
}