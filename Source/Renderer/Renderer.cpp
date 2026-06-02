#include "pch.h"
#include "Renderer.h"
#include "ShaderTypes.h"
#include "Graphics/Graphics.h"
#include "Graphics/RenderPass.h"
#include "Graphics/PipelineState.h"
#include "Scene/Scene.h"
#include "Shader/ShaderManager.h"
#include "Shader/ShaderProgram.h"
#include "Scene/Components/MeshRenderer.h"
#include "Scene/Components/Camera.h"
#include "Scene/Components/Transform.h"
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

			// 일단 셰이더 바인딩 후 상수버퍼 바인딩이라는 순서를 맞추기 위해 옮겼지만
			// Drawable마다 바인딩할 필요가 없는 데이터다.
			{
				CameraData cameraData{};
				cameraData.viewMatrix = DirectX::XMMatrixTranspose(camera->GetViewMatrix());
				cameraData.projMatrix = DirectX::XMMatrixTranspose(camera->GetProjectionMatrix());
				cameraData.viewProjMatrix = DirectX::XMMatrixTranspose(camera->GetViewProjMatrix());
				auto& pos = camera->GetTransform()->GetPosition();
				cameraData.position = DirectX::XMFLOAT4(pos.x, pos.y, pos.z, 1.0f);
				//cameraData.backgroundColor = camera->GetBackgroundColor();

				// Graphics가 가진 b0 전역 버퍼에 데이터를 밀어 넣고 바인딩합니다.
				m_graphics->UpdateConstantBuffer(eCBufferSlot::Camera, &cameraData, sizeof(cameraData));
				m_graphics->BindConstantBuffer(eCBufferSlot::Camera);
			}

			for (GameObject* drawable : scene->GetDrawable())
			{
				auto* transform = drawable->GetTransform();
				auto* meshRenderer = drawable->GetComponent<MeshRenderer>();
				auto mat = meshRenderer->GetMaterial();
				auto mesh = meshRenderer->GetMesh();

				{
					ObjectVS objData{};
					objData.worldMatrix = DirectX::XMMatrixTranspose(transform->GetTransformMatrix());
					m_graphics->UpdateConstantBuffer(eCBufferSlot::Object, &objData, sizeof(objData));
					m_graphics->BindConstantBuffer(eCBufferSlot::Object);
				}

				{
					PipelineState state{};
					auto shaderProgram = ShaderManager::GetInst().GetProgram(mat->GetShader());
					state.vertexShader = shaderProgram->GetVertexShader();
					state.pixelShader = shaderProgram->GetPixelShader();
					state.inputLayout = shaderProgram->GetInputLayout();	// 그냥 ShaderProgram으로 받는 게 낫지 않을까?
					state.depthStencilState = eDepthStencilState::DepthReadWrite;
					state.rasterizerState = eRasterizerState::FillSolid_CullBack;
					state.topology = mesh->GetTopology();

					m_graphics->SetPipelineState(state);
				}

				{
					// 이건 mesh의 bind에서 처리하라고...?
					m_graphics->BindVertexBuffer(mesh->GetVertexBuffer());
					m_graphics->BindIndexBuffer(mesh->GetIndexBuffer());

					m_graphics->DrawIndexed(mesh->GetIndexCount());
				}
			}
			m_graphics->EndRenderPass();
		}
	}
}