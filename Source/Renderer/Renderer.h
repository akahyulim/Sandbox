#pragma once
#include <d3d11.h>
#include "Graphics/PipelineState.h"
#include "Graphics/Graphics.h"
#include "Graphics/ShaderType.h"

namespace Dive
{
	//class Graphics;
	class VertexBuffer;
	class IndexBuffer;
	class Material;
	class StateicMesh;
	struct RenderPass;
	class Scene;
	class GameObject;
	class MeshRenderer;

	// Grahics의 일부 기능을 이 곳으로 옮기자.
	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		bool Initialize(Graphics* graphics);

		void Update(Scene* scene);
		void Render(Scene* scene);

		void SetPipelineState(const PipelineState& pso, uint32_t stencilRef, float blendFactor[4], uint32_t sampleMask);

		void UpdateCameraBuffer(const cbCamera& data);
		void UpdateMaterialBuffer(const cbMaterial& data);
		void UpdateObjectBuffer(const cbObject& data);
		void UpdateLightBuffer(const cbLight& data);

		void DrawMesh(MeshRenderer* mr);

	private:
		void BindVertexBuffer(VertexBuffer* vb);
		void BindIndexBuffer(IndexBuffer* ib);

		void UpdateConstantBuffer(eCBufferSlot slot, const void* data, uint32_t size);

	private:
		Graphics* m_graphics = nullptr;

		std::vector<GameObject*> m_cameras;
		std::vector<GameObject*> m_lights;
		std::vector<GameObject*> m_drawables;

		PipelineState m_currentPSO;
		uint32_t m_currentStencilRef = 0;
		float m_currentBlendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		uint32_t m_currentSampleMask = 0xFFFFFFFF;

		// 여기서부터 다시 구현하자.
		uint32_t m_currentRTVCount = 0;
		ID3D11RenderTargetView* m_currentRTVs[8] = { nullptr };
		ID3D11DepthStencilView* m_currentDSV = nullptr;

		VertexBuffer* m_currentVB = nullptr;
		IndexBuffer* m_currentIB = nullptr;

		cbCamera m_currentCameraData{};
		cbMaterial m_currentMaterialData{};
		cbObject m_currentObjectData{};
		cbLight m_currentLightData{};

		Viewport m_currentViewport;
	};
}