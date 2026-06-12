#include "pch.h"
#include "Renderer.h"
#include "Graphics/ShaderType.h"
#include "Graphics/Graphics.h"
#include "Graphics/RenderPass.h"
#include "Graphics/PipelineState.h"
#include "Scene/Scene.h"
#include "Shader/ShaderManager.h"
#include "Shader/ShaderProgram.h"
#include "Scene/Components/MeshRenderer.h"
#include "Scene/Components/Camera.h"
#include "Scene/Components/Transform.h"
#include "Scene/Components/Light.h"
#include "Resource/Material.h"
#include "Resource/StaticMesh.h"

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

	void Renderer::Render(Scene* scene)
	{
		if (scene == nullptr)
			return;

		if (Camera* camera = scene->GetMainCamera()->GetComponent<Camera>())
		{
			RenderPass finalPass{};
			finalPass.clearColor = scene->GetClearColor();	// camera에서 가지는 게 나을 듯?
			finalPass.count = 1;
			finalPass.rtvs[0] = m_graphics->GetRenderTargetView();
			finalPass.dsv = m_graphics->GetDepthStencilView();
			finalPass.viewport = camera->GetViewport();
			m_graphics->BeginRenderPass(finalPass);

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
				auto& dirLightData = scene->GetDirectionalLight()->GetComponent<Light>()->GetLightData();
				m_graphics->UpdateConstantBuffer(eCBufferSlot::Light, &dirLightData, sizeof(dirLightData));
				m_graphics->BindConstantBuffer(eCBufferSlot::Light);
			}

			for (GameObject* drawable : scene->GetDrawable())
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
			m_graphics->EndRenderPass();
		}
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