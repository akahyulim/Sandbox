#include "stdafx.h"
#include "Camera.h"
#include "Transform.h"
#include "../GameObject.h"
//#include "Core/CoreSystem.h"
#include "Graphics/GraphicsSystem.h"
#include "Resource/ResourceSystem.h"

#include "Common/Math.h"

using namespace DirectX;

namespace Dive
{
	static constexpr float FOV_MIN = 1.0f;
	static constexpr float FOV_MAX = 160.0f;

	static constexpr float NEAR_CLIP_PLANE = 0.1f;

	std::vector<Camera*> Camera::s_allCameras;

	Camera::Camera(GameObject* owner, uint64_t id)
		: Component(owner, id)
	{
		s_allCameras.push_back(this);

		//m_cbCamera = std::make_unique<ConstantBuffer>(static_cast<uint32_t>(sizeof(CameraData)));
		//if (!m_cbCamera) DV_LOG(MeshRenderer, eLogLevel::Err, "[::Camera] cbCamera 생성 실패");

		m_viewport.TopLeftX = 0.0f;
		m_viewport.TopLeftY = 0.0f;
		m_viewport.Width = static_cast<float>(GraphicsDevice::GetWidth());
		m_viewport.Height = static_cast<float>(GraphicsDevice::GetHeight());
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;
	}

	Camera::~Camera()
	{
		auto it = std::find(s_allCameras.begin(), s_allCameras.end(), this);
		if (it != s_allCameras.end())
			s_allCameras.erase(it);
	}

	bool Camera::IsEqualTo(const Component* other) const
	{
		auto target = dynamic_cast<const Camera*>(other);
		if (!target) return false;

		return (m_projectionType == target->m_projectionType) &&
			(m_fov == target->m_fov) &&
			(m_nearClip == target->m_nearClip) &&
			(m_farClip == target->m_farClip) &&
			(m_viewportTop == target->m_viewportTop) &&
			(m_viewportLeft == target->m_viewportLeft) &&
			(m_viewportRight == target->m_viewportRight) &&
			(m_viewportBottom == target->m_viewportBottom) &&
			Math::XMFLOAT4Equal(m_backgroundColor, target->m_backgroundColor) &&
			(m_renderTarget == target->m_renderTarget);
	}

	std::unique_ptr<Component> Camera::Clone(GameObject* owner) const
	{
		auto clone = std::make_unique<Camera>(owner);
		
		clone->m_projectionType = m_projectionType;
		clone->m_fov = m_fov;
		clone->m_nearClip = m_nearClip;
		clone->m_farClip = m_farClip;
		clone->m_viewportTop = m_viewportTop;
		clone->m_viewportLeft = m_viewportLeft;
		clone->m_viewportRight = m_viewportRight;
		clone->m_viewportBottom = m_viewportBottom;
		clone->m_viewport = m_viewport;
		clone->m_frustum = m_frustum;
		clone->m_backgroundColor = m_backgroundColor;
		clone->m_renderTarget = nullptr;
		
		return clone;
	}

	void Camera::Serialize(YAML::Emitter& out)
	{
		if (!m_isDirty)
			return;

		out << YAML::Key << "Camera" << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "ID" << YAML::Value << GetInstanceID();
		out << YAML::Key << "ProjectionType" << YAML::Value << static_cast<int>(m_projectionType);
		out << YAML::Key << "FieldOfView" << YAML::Value << m_fov;
		out << YAML::Key << "NearClipPlane" << YAML::Value << m_nearClip;
		out << YAML::Key << "FarClipPlane" << YAML::Value << m_farClip;
		out << YAML::Key << "ViewportLeft" << YAML::Value << m_viewportLeft;
		out << YAML::Key << "ViewportTop" << YAML::Value << m_viewportTop;
		out << YAML::Key << "ViewportRight" << YAML::Value << m_viewportRight;
		out << YAML::Key << "ViewportBottom" << YAML::Value << m_viewportBottom;
		out << YAML::Key << "BackgroundColor" << YAML::Value << m_backgroundColor;
		out << YAML::EndMap;

		m_isDirty = false;
	}

	void Camera::Deserialize(const YAML::Node& node)
	{
		if (GetInstanceID() != node["ID"].as<uint64_t>())
		{
			DV_LOG(GameObject, eLogLevel::Err, "[::Deserialize] InstanceID 초기화에 실패하였습니다.");
			return;
		}

		m_projectionType = static_cast<eProjectionType>(node["ProjectionType"].as<int>());
		m_fov = node["FieldOfView"].as<float>();
		m_nearClip = node["NearClipPlane"].as<float>();
		m_farClip = node["FarClipPlane"].as<float>();
		m_viewportLeft = node["ViewportLeft"].as<float>();
		m_viewportTop = node["ViewportTop"].as<float>();
		m_viewportRight = node["ViewportRight"].as<float>();
		m_viewportBottom = node["ViewportBottom"].as<float>();
		m_backgroundColor = node["BackgroundColor"].as<XMFLOAT4>();

		m_isDirty = true;
	}

	void Camera::Update()
	{
		if (!m_owner || !m_owner->IsActiveSelf())
			return;

		// 이 부분을 건너뛰니 프러스텀 갱신이 안됐다.
		//if (!GetTransform()->WasUpdated())
		//	return;

		m_frustum.Update(GetViewMatrix(), GetProjectionMatrix());
	}

	DirectX::XMFLOAT4X4 Camera::GetView() const
	{
		XMFLOAT4X4 view{};
		XMStoreFloat4x4(&view, GetViewMatrix());
		return view;
	}

	DirectX::XMMATRIX Camera::GetViewMatrix() const
	{
		auto transform = GetTransform();
		assert(transform);

		return XMMatrixLookToLH(transform->GetPositionVector(), transform->GetForwardVector(), transform->GetUpVector());
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
				m_farClip);
		}
		else
		{		
			// 코파일럿이 작성한  코드다.
			// width, height가 맞는지 모르겠다.
			return XMMatrixOrthographicLH(
				GetAspectRatio() * m_farClip,
				m_farClip,
				m_nearClip,
				m_farClip);
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
		return (m_viewport.Width / m_viewport.Height);
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
			DV_LOG(Camera, eLogLevel::Warn, "[::SetNearClipPlane] 유효하지 않은 Near Clip 값: {}", nearPlane);
			return;
		}
		m_nearClip = nearPlane;
	}

	void Camera::SetFarClipPlane(float farPlane)
	{
		if (farPlane <= m_nearClip)
		{
			DV_LOG(Camera, eLogLevel::Warn, "유효하지 않은 Far Clip 값 전달: {}", farPlane);
			return;
		}
		m_farClip = farPlane;
	}

	void Camera::GetViewportRect(float& outLeft, float& outTop, float& outRight, float& outBottom) const
	{
		outLeft = m_viewportLeft;
		outTop = m_viewportTop;
		outRight = m_viewportRight;
		outBottom = m_viewportBottom;
	}

	void Camera::SetViewportRect(float left, float top, float right, float bottom)
	{
		if (left < 0.0f || left > right ||
			top < 0.0f || top > bottom ||
			right <= left || right > 1.0f ||
			bottom <= top || bottom > 1.0f)
		{
			DV_LOG(Camera, eLogLevel::Warn, "[::SetViewportRect] 유효하지 않은 뷰포트 설정값: {}, {}, {}, {}", left, top, right, bottom);
			return;
		}

		m_viewportLeft = left;
		m_viewportTop = top;
		m_viewportRight = right;
		m_viewportBottom = bottom;
	}

	void Camera::SetViewportTop(float top)
	{
		if (top < 0.0f || top > m_viewportBottom)
		{
			DV_LOG(Camera, eLogLevel::Warn, "[::SetViewportTop] 유효하지 않은 값: {}", top);
			return;
		}
		m_viewportTop = top;
	}

	void Camera::SetViewportLeft(float left)
	{
		if (left < 0.0f || left > m_viewportRight)
		{
			DV_LOG(Camera, eLogLevel::Warn, "[::SetViewportLeft] 유효하지 않은 값: {}", left);
			return;
		}
		m_viewportLeft = left;
	}

	void Camera::SetViewportRight(float right)
	{
		if (right < m_viewportLeft || right > 1.0f)
		{
			DV_LOG(Camera, eLogLevel::Warn, "[::SetViewportRight] 유효하지 않은 값: {}", right);
			return;
		}
		m_viewportRight = right;
	}

	void Camera::SetViewportBottom(float bottom)
	{
		if (bottom < m_viewportTop || bottom > 1.0f)
		{
			DV_LOG(Camera, eLogLevel::Warn, "[::SetViewportBottom] 유효하지 않은 값: {}", bottom);
			return;
		}
		m_viewportBottom = bottom;
	}

	void Camera::SetViewport(float topLeftX, float topLeftY, float width, float height, float minDepth, float maxDepth)
	{
		m_viewport.TopLeftX = topLeftX;
		m_viewport.TopLeftY = topLeftY;
		m_viewport.Width = width;
		m_viewport.Height = height;
		m_viewport.MinDepth = minDepth;
		m_viewport.MaxDepth = maxDepth;
	}

	D3D11_VIEWPORT* Camera::GetViewport()
	{
		return &m_viewport;
	}

	void Camera::SetBackgroundColor(float r, float g, float b, float a)
	{
		m_backgroundColor = { r,g,b,a };
	}

	RenderTexture* Camera::GetRenderTarget() const
	{
		return m_renderTarget ? m_renderTarget : nullptr;
	}

	ID3D11RenderTargetView* Camera::GetRenderTargetView() const
	{
		return m_renderTarget ? 
			m_renderTarget->GetRenderTargetView() : 
			GraphicsDevice::GetRenderTargetView();
	}

	ID3D11DepthStencilView* Camera::GetDepthStencilView() const
	{
		return m_renderTarget ?
			m_renderTarget->GetDepthStencilView() : 
			GraphicsDevice::GetDepthStencilView();
	}


	void Camera::SetRenderTarget(RenderTexture* renderTarget)
	{
		m_renderTarget = renderTarget;

		SetViewport(0.0f, 0.0f,
			static_cast<float>(renderTarget ? m_renderTarget->GetWidth() : GraphicsDevice::GetWidth()),
			static_cast<float>(renderTarget ? m_renderTarget->GetHeight() : GraphicsDevice::GetHeight()));
	}

	Ray Camera::ScreenPointToRay(const DirectX::XMUINT2& mousePosition) const
	{
		return ScreenPointToRay(mousePosition.x, mousePosition.y);
	}

	Ray Camera::ScreenPointToRay(uint32_t mousePositionX, uint32_t mousePositionY) const
	{
		// 1. 마우스 좌표 → NDC 변환
		float x = (2.0f * mousePositionX) / GetRenderTarget()->GetWidth() - 1.0f;
		float y = 1.0f - (2.0f * mousePositionY) / GetRenderTarget()->GetHeight();

		XMVECTOR ndcNear = XMVectorSet(x, y, 0.0f, 1.0f);
		XMVECTOR ndcFar = XMVectorSet(x, y, 1.0f, 1.0f);

		// 2. ViewProjection 역행렬 계산
		XMMATRIX viewProj =  GetViewMatrix() * GetProjectionMatrix();
		XMMATRIX invViewProj = XMMatrixInverse(nullptr, viewProj);

		// 3. NDC → 월드 공간 변환
		XMVECTOR worldNearVec = XMVector4Transform(ndcNear, invViewProj);
		XMVECTOR worldFarVec = XMVector4Transform(ndcFar, invViewProj);

		XMFLOAT4 worldNear, worldFar;
		XMStoreFloat4(&worldNear, worldNearVec);
		XMStoreFloat4(&worldFar, worldFarVec);

		XMFLOAT3 origin = {
			worldNear.x / worldNear.w,
			worldNear.y / worldNear.w,
			worldNear.z / worldNear.w
		};

		XMFLOAT3 farPoint = {
			worldFar.x / worldFar.w,
			worldFar.y / worldFar.w,
			worldFar.z / worldFar.w
		};

		XMFLOAT3 direction = {
			farPoint.x - origin.x,
			farPoint.y - origin.y,
			farPoint.z - origin.z
		};

		XMStoreFloat3(&direction, XMVector3Normalize(XMLoadFloat3(&direction)));

		return Ray(origin, direction);
	}


	Camera* Camera::GetMainCamera()
	{
		for (auto pCamera : s_allCameras)
		{
			if (pCamera->GetGameObject()->GetTag() == "MainCamera")
				return pCamera;
		}

		return nullptr;
	}
}
