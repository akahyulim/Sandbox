#include "pch.h"
#include "Renderer.h"
#include "Graphics/ShaderType.h"
//#include "Graphics/Graphics.h"
#include "Graphics/RenderPass.h"
//#include "Graphics/PipelineState.h"
#include "Scene/Scene.h"
#include "Resource/ShaderManager.h"
#include "Resource/ShaderProgram.h"
#include "Scene/Components/MeshRenderer.h"
#include "Scene/Components/Camera.h"
#include "Scene/Components/Transform.h"
#include "Scene/Components/Light.h"
#include "Scene/GameObject.h"
#include "Resource/Material.h"
#include "Resource/StaticMesh.h"
#include "Resource/RenderTexture.h"

namespace Dive
{
	Renderer::Renderer()
	{
	}
	
	Renderer::~Renderer()
	{
	}

	bool Renderer::Initialize(Graphics* graphics)
	{
		assert(graphics != nullptr);
		m_graphics = graphics;
		// 초기화 완료 확인
		return true;
	}

	void Renderer::Update(Scene* scene)
	{
		if (!scene->IsDirty())
			return;

		m_cameras.clear();
		m_lights.clear();
		m_drawables.clear();

		for (auto go : scene->GetAll())
		{
			if (!go->IsActive())
				continue;

			if (auto camera = go->GetComponent<Camera>())
				m_cameras.push_back(go);
			if (auto light = go->GetComponent<Light>())
				m_lights.push_back(go);
			if (auto meshRenderer = go->GetComponent<MeshRenderer>())
				m_drawables.push_back(go);
		}
		
		// 컬링
		{

		}

		scene->ClearDirty();
	}

	void Renderer::Render(Scene* scene)
	{
		if (scene == nullptr)
			return;

		ID3D11ShaderResourceView* srv = nullptr;

		for (auto go : m_cameras)
		{
			auto camera = go->GetComponent<Camera>();

			RenderPass opaquePass{};
			opaquePass.clearColor = camera->GetClearColor();
			opaquePass.count = 1;
			opaquePass.rtvs[0] = camera->GetTargetTexture() ? camera->GetTargetTexture()->GetRenderTargetView() : m_graphics->GetRenderTargetView();
			opaquePass.dsv = camera->GetTargetTexture() ? camera->GetTargetTexture()->GetDepthStencilView() : m_graphics->GetDepthStencilView();
			{
				auto width = m_graphics->GetWidth();
				auto height = m_graphics->GetHeight();
				Viewport viewport;
				viewport.width = (float)width;
				viewport.height = (float)height;
				viewport.maxDepth = 1.0f;
				opaquePass.viewport = viewport;// camera->GetViewport();
			}
			m_graphics->BeginRenderPass(opaquePass);

			m_graphics->BindAllSamplers();

			// 일단 셰이더 바인딩 후 상수버퍼 바인딩이라는 순서를 맞추기 위해 옮겼지만
			// Drawable마다 바인딩할 필요가 없는 데이터다.
			// => BindCameraConstantBuffer(camera); 처럼 메서드로 만들어놓고 사용하도록 제안하고 있다.
			{
				cbCamera cameraData{};
				cameraData.viewMatrix = DirectX::XMMatrixTranspose(camera->GetViewMatrix());
				cameraData.projMatrix = DirectX::XMMatrixTranspose(camera->GetProjectionMatrix());
				cameraData.viewProjMatrix = DirectX::XMMatrixTranspose(camera->GetViewProjMatrix());
				auto pos = camera->GetTransform()->GetPosition();
				cameraData.position = DirectX::XMFLOAT4(pos.x, pos.y, pos.z, 1.0f);
				//cameraData.backgroundColor = camera->GetBackgroundColor();

				// Graphics가 가진 b0 전역 버퍼에 데이터를 밀어 넣고 바인딩합니다.
				m_graphics->UpdateConstantBuffer(eCBufferSlot::Camera, &cameraData, sizeof(cameraData));
				m_graphics->BindConstantBuffer(eCBufferSlot::Camera);
			}

			// dir light
			{
				auto dirLightGO = m_lights[0];
				auto& dirLightData = dirLightGO->GetComponent<Light>()->GetLightData();
				m_graphics->UpdateConstantBuffer(eCBufferSlot::Light, &dirLightData, sizeof(dirLightData));
				m_graphics->BindConstantBuffer(eCBufferSlot::Light);
			}

			for (GameObject* drawable : m_drawables)
			{
				auto* transform = drawable->GetTransform();
				auto* meshRenderer = drawable->GetComponent<MeshRenderer>();
				auto mat = meshRenderer->GetMaterial();
				auto mesh = meshRenderer->GetMesh();

				// UpdateObjectConstantBuffer(transform); 역시 이렇게 메서드 활용을 제안하고 있다.
				{
					cbObject objData{};
					objData.worldMatrix = DirectX::XMMatrixTranspose(transform->GetWorldMatrix());
					m_graphics->UpdateConstantBuffer(eCBufferSlot::Object, &objData, sizeof(objData));
					m_graphics->BindConstantBuffer(eCBufferSlot::Object);
				}

				{
					mat->Bind(m_graphics);
				}

				{
					PipelineState state{};
					state.topology = mesh->GetTopology(); 
					state.shaderProgram = mat->GetShaderProgram();
					state.depthStencilState = eDepthStencilState::DepthReadWrite;
					state.rasterizerState = eRasterizerState::FillSolid_CullBack;
					m_graphics->SetPipelineState(state);
				}

				mesh->Bind(m_graphics);

				m_graphics->DrawIndexed(mesh->GetIndexCount());
				
			}

			srv = camera->GetTargetTexture()->GetShaderResourceView();

			m_graphics->EndRenderPass();
		}


		// Resolve pass
		{
			RenderPass finalPass{};
			finalPass.clearColor = Color::White;
			finalPass.count = 1;
			finalPass.rtvs[0] = m_graphics->GetRenderTargetView();
			finalPass.dsv = m_graphics->GetDepthStencilView();
			{
				auto width = m_graphics->GetWidth();
				auto height = m_graphics->GetHeight();
				Viewport viewport;
				viewport.width = (float)width;
				viewport.height = (float)height;
				viewport.maxDepth = 1.0f;
				finalPass.viewport = viewport;// camera->GetViewport();
			}
			m_graphics->BeginRenderPass(finalPass);

			m_graphics->GetDeviceContext()->PSSetShaderResources(30, 1, &srv);

			PipelineState state{};
			state.topology = ePrimitiveTopology::TriangleStrip;
			state.shaderProgram = ShaderManager::GetInst().GetProgram("ResolveScene");
			state.depthStencilState = eDepthStencilState::DepthReadWrite;
			state.rasterizerState = eRasterizerState::FillSolid_CullBack;
			m_graphics->SetPipelineState(state);

			m_graphics->GetDeviceContext()->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
			m_graphics->Draw(4, 0);
		}
	}

	void Renderer::SetPipelineState(const PipelineState& pso, uint32_t stencilRef, float blendFactor[4], uint32_t sampleMask)
	{
		if (m_currentPSO == pso &&
			m_currentStencilRef == stencilRef &&
			m_currentBlendFactor[0] == blendFactor[0] &&
			m_currentBlendFactor[1] == blendFactor[1] &&
			m_currentBlendFactor[2] == blendFactor[2] &&
			m_currentBlendFactor[3] == blendFactor[3] &&
			m_currentSampleMask == sampleMask)
			return;

		assert(m_graphics);

		m_graphics->BindPipelineState(pso, stencilRef, blendFactor, sampleMask);

		m_currentPSO = pso;
		m_currentStencilRef = stencilRef;
		m_currentBlendFactor[0] = blendFactor[0];
		m_currentBlendFactor[1] = blendFactor[1];
		m_currentBlendFactor[2] = blendFactor[2];
		m_currentBlendFactor[3] = blendFactor[3];
		m_currentSampleMask = sampleMask;
	}

	void Renderer::UpdateCameraBuffer(const cbCamera& data)
	{
		if (memcmp(&m_currentCameraData, &data, sizeof(cbCamera)) == 0)
			return;

		UpdateConstantBuffer(eCBufferSlot::Camera, &data, sizeof(data));

		m_currentCameraData = data;
	}

	void Renderer::UpdateMaterialBuffer(const cbMaterial& data)
	{
		if (memcmp(&m_currentMaterialData, &data, sizeof(cbMaterial)) == 0)
			return;

		UpdateConstantBuffer(eCBufferSlot::Material, &data, sizeof(data));

		m_currentMaterialData = data;
	}

	void Renderer::UpdateObjectBuffer(const cbObject& data)
	{
		if (memcmp(&m_currentObjectData, &data, sizeof(cbObject)) == 0)
			return;

		UpdateConstantBuffer(eCBufferSlot::Object, &data, sizeof(data));

		m_currentObjectData = data;
	}

	void Renderer::UpdateLightBuffer(const cbLight& data)
	{
		if (memcmp(&m_currentLightData, &data, sizeof(cbLight)) == 0)
			return;

		UpdateConstantBuffer(eCBufferSlot::Light, &data, sizeof(data));

		m_currentLightData = data;
	}

	void Renderer::DrawMesh(MeshRenderer* mr)
	{
		if (mr == nullptr)
			return;

		// 예시에선 raw ptr로 받았다.
		auto mesh = mr->GetMesh();
		auto mat = mr->GetMaterial();

		// pso를 Resource로 다루는 게 나을 것 같다.
		//BindPipelineState(mat->)
		
		//UpdateObjectBuffer(mr->)

		BindVertexBuffer(mesh->GetVertexBuffer());
		BindIndexBuffer(mesh->GetIndexBuffer());

		m_graphics->DrawIndexed(mesh->GetIndexCount());
	}

	void Renderer::BindVertexBuffer(VertexBuffer* vb)
	{
		if (m_currentVB == vb)
			return;

		assert(m_graphics);

		m_graphics->BindVertexBuffer(vb);
		m_currentVB = vb;
	}

	void Renderer::BindIndexBuffer(IndexBuffer* ib)
	{
		if (m_currentIB == ib)
			return;

		assert(m_graphics);

		m_graphics->BindIndexBuffer(ib);
		m_currentIB = ib;
	}

	void Renderer::UpdateConstantBuffer(eCBufferSlot slot, const void* data, uint32_t size)
	{
		assert(m_graphics);

		m_graphics->UpdateConstantBuffer(slot, data, size);
	}

	/*
	수정 예시 코드
	void Renderer::Render(Scene* scene)
	{
		// 🌟 1. 불투명 / 반투명 소팅 및 패스 제어 (앞서 구축한 구조)
		m_graphics->BeginRenderPass(finalPass);
		m_graphics->BindAllSamplers();

		// 🌟 2. 전역 카메라 데이터 바인딩 (수정 불필요, 완벽함!)
		BindCameraConstantBuffer(camera); 

		// 🌟 3. 오브젝트 루프 (극단적인 다이어트 성공)
		for (GameObject* drawable : scene->GetDrawable())
		{
			auto* transform = drawable->GetTransform();
			auto* meshRenderer = drawable->GetComponent<MeshRenderer>();
			auto mat = meshRenderer->GetMaterial();
			auto mesh = meshRenderer->GetMesh();

			// ① [Transform] 오브젝트 행렬 업데이트 및 바인딩
			UpdateObjectConstantBuffer(transform);

			// ② [Material] 마티리얼이 알아서 셰이더, 텍스처, 자체 cbuffer 바인딩
			mat->Bind(m_graphics); 

			// ③ [Pipeline] 마티리얼의 상태를 기반으로 파이프라인 최종 세팅
			// (여기서 mat->GetShader() 또는 mat->GetRenderQueue()에 따른 깊이/래스터라이저 상태 조합)
			PipelineState state = mat->GetPipelineState(); 
			state.topology = mesh->GetTopology(); // 토폴로지만 메쉬에서 공급
			m_graphics->SetPipelineState(state);

			// ④ [Mesh] 메쉬가 알아서 VB, IB 바인딩
			mesh->Bind(m_graphics);

			// ⑤ [Draw] 최종 드로우 명령
			m_graphics->DrawIndexed(mesh->GetIndexCount());
		}

		m_graphics->EndRenderPass();
	}
	*/
}