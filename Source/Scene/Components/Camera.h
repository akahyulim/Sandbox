#pragma once
#include <yaml-cpp/yaml.h>
#include <DirectXMath.h>
#include <d3d11.h>
#include "Component.h"
#include "Rendering/Frustum.h"

namespace Dive
{
	class RenderTexture;
	class ConstantBuffer;

	enum class eProjectionType
	{
		Perspective,
		Orthographic
	};

	struct Ray
	{
		Ray(const DirectX::XMFLOAT3& org, const DirectX::XMFLOAT3 dir)
			: origin(org)
			, direction(dir)
		{}

		DirectX::XMFLOAT3 origin;
		DirectX::XMFLOAT3 direction;
	};

	struct RaycastHit
	{
		GameObject* hitObject;
		DirectX::XMFLOAT3 point;
		DirectX::XMFLOAT3 normal;
		float distance;
	};

	// https://docs.unity3d.com/ScriptReference/Camera.html
	class Camera : public Component
	{
	public:
		Camera(GameObject* owner, uint64_t id = AUTO_ID);
		~Camera()override;

		bool IsEqualTo(const Component* other) const override;

		std::unique_ptr<Component> Clone(GameObject* owner) const override;

		void Serialize(YAML::Emitter& out) override;
		void Deserialize(const YAML::Node& node) override;

		void Update() override;

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

		void GetViewportRect(float& outLeft, float& outTop, float& outRight, float& outBottom) const;
		float GetViewportLeft() const { return m_viewportLeft; }
		float GetViewportTop() const { return m_viewportTop; }
		float GetViewportRight() const { return m_viewportRight; }
		float GetViewportBottom() const { return m_viewportBottom; }
		void SetViewportRect(float left, float top, float right, float bottom);
		void SetViewportTop(float top);
		void SetViewportLeft(float left);
		void SetViewportRight(float right);
		void SetViewportBottom(float bottom);
		
		void SetViewport(float topLeftX, float topLeftY, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f);
		D3D11_VIEWPORT* GetViewport();

		const Frustum& GetFrustum() const { return m_frustum; }

		const DirectX::XMFLOAT4& GetBackgroundColor() const { return m_backgroundColor; }
		void SetBackgroundColor(float r, float g, float b, float a);
		void SetBackgroundColor(const DirectX::XMFLOAT4& color) { m_backgroundColor = color; }

		RenderTexture* GetRenderTarget() const;
		ID3D11RenderTargetView* GetRenderTargetView() const;
		ID3D11DepthStencilView* GetDepthStencilView() const;
		void SetRenderTarget(RenderTexture* renderTarget);

		Ray ScreenPointToRay(const DirectX::XMUINT2& mousePosition) const;
		Ray ScreenPointToRay(uint32_t mousePositionX, uint32_t mousePositionY) const;

		static constexpr eComponentType GetComponentType() { return eComponentType::Camera; }

		static std::vector<Camera*> GetAllCameras() { return s_allCameras; }
		static uint32_t GetAllCameraCount() { return static_cast<uint32_t>(s_allCameras.size()); }
		static Camera* GetMainCamera();

	private:
		eProjectionType m_projectionType = eProjectionType::Perspective;
				
		float m_fov = 45.0f;

		float m_nearClip = 0.1f;
		float m_farClip = 5000.0f;

		float m_viewportTop = 0.0f;
		float m_viewportLeft = 0.0f;
		float m_viewportRight = 1.0f;
		float m_viewportBottom = 1.0f;

		D3D11_VIEWPORT m_viewport{};

		Frustum m_frustum{};

		DirectX::XMFLOAT4 m_backgroundColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		RenderTexture* m_renderTarget = nullptr;

		static std::vector<Camera*> s_allCameras;
	};
}
