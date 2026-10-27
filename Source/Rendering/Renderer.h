#pragma once
#include <wrl/client.h>
#include <memory>
#include <array>
#include <d3d11.h>

#include "Graphics/ConstantBuffer.h"
#include "Graphics/ConstantBufferDatas.h"
#include "Graphics/StructuredBuffer.h"
#include "Graphics/RenderPass.h"

namespace Dive
{
	class Graphics;
	class RenderTexture;
	class VertexBuffer;
	class IndexBuffer;
	class Scene;
	class GameObject;

	class Renderer
	{
		enum class eDepthStencilState : uint8_t
		{
			Default,
			StencilMark,
			GBuffer,
			DepthDisabled,
			ForwardLight,
			Transparent,
			Skybox,
			Count
		};

		enum class eRasterizerState : uint8_t
		{
			FillSolid_CullFront,
			FillSolid_CullBack,
			FillSolid_CullNone,
			Count
		};

		enum class eBlendState : uint8_t
		{
			Forward,
			Count
		};

		enum class eSamplerState : uint8_t
		{
			WrapLinear,
			ClampPoint,
			ClampLinear,
			Skybox,
			SkySphere,
			ShadowCompare,
			Count
		};

		enum class eGBufferType : uint8_t
		{
			AlbedoRoughness = 0, // Albedo (RGB) + Roughness (A)
			NormalMetallic,      // View Space Normal (RGB) + Metallic (A)
			Emissive,            // Emissive (RGB)
			ObjectID,
			Depth,
			Count
		};

	public:
		explicit Renderer(Graphics* graphics, uint32_t width, uint32_t height);
		~Renderer();

		void Update(Scene* scene);
		void Render(Scene* scene);

		void ResolveToOffScreenTexture();
		void ResolveToBackbuffer();

		void OnResize(uint32_t width, uint32_t height);

		const RenderTexture* GetOffScreenTexture() const { return m_offScreenRenderTarget.get(); }

		void SetMousePosition(const DirectX::XMUINT2& cursorPos);

		void SetSelectedObjectID(uint32_t objectID);

		void ProcessPicking();
		PickingData GetPickingData() const { return m_pickingData; }

	private:
		void createDepthStencilStates();
		void createRasterizerStates();
		void createBlendStates();
		void createSamplers();
		void createBuffers();
		void createResolutionDependantResources(uint32_t width, uint32_t height);

		void createRenderTargets(uint32_t width, uint32_t height);
		void createGBuffer(uint32_t width, uint32_t height);
		void createRenderPasses(uint32_t width, uint32_t height);

		void loadTextures();

		void bindGlobals();
		
		void updateEnvironment(Scene* scene);

		void passGBuffer();
		void passLighting();
		void passSky(Scene* scene);
		void passForward();
		void passPostProcessing();

	private:
		Graphics* m_graphics = nullptr;
		uint32_t m_width;
		uint32_t m_height;

		std::array<Microsoft::WRL::ComPtr<ID3D11DepthStencilState>, static_cast<size_t>(eDepthStencilState::Count)> m_depthStencilStates;
		std::array<Microsoft::WRL::ComPtr<ID3D11RasterizerState>, static_cast<size_t>(eRasterizerState::Count)>m_rasterizerStates;
		std::array<Microsoft::WRL::ComPtr<ID3D11BlendState>, static_cast<size_t>(eBlendState::Count)> m_blendStates;
		std::array<Microsoft::WRL::ComPtr<ID3D11SamplerState>, static_cast<size_t>(eSamplerState::Count)> m_samplerStates;
		
		// render targets
		std::array<std::unique_ptr<RenderTexture>, static_cast<size_t>(eGBufferType::Count)> m_gbuffer;

		std::unique_ptr<RenderTexture> m_hdrRenderTarget;
		std::unique_ptr<RenderTexture> m_ldrRenderTarget;
		std::unique_ptr<RenderTexture> m_offScreenRenderTarget;
		std::unique_ptr<RenderTexture> m_depthTarget;

		std::unique_ptr<ConstantBuffer<FrameData>> m_cbFrame;
		std::unique_ptr<ConstantBuffer<ObjectData>> m_cbObject;
		std::unique_ptr<ConstantBuffer<EnvironmentData>> m_cbWeather;
		std::unique_ptr<ConstantBuffer<LightConstants>> m_cbLight;

		uint32_t m_lastSelectedObjectID = 0;
		std::unique_ptr<ConstantBuffer<SelectedObjectData>> m_cbSelectedObject;

		PickingData m_pickingData{};
		std::unique_ptr<StructuredBuffer<PickingData>> m_pickingBuffer;

		RenderPassDesc m_gbufferPass;
		RenderPassDesc m_deferredLightingPass;
		RenderPassDesc m_forwardPass;
		RenderPassDesc m_offScreenResolvePass;
		RenderPassDesc m_skyboxPass;	// 원래는 forward를 사용

		std::unique_ptr<VertexBuffer> m_cubeVB;
		std::unique_ptr<IndexBuffer> m_cubeIB;

		DirectX::XMFLOAT2 m_mousePosition;

		std::vector<GameObject*> m_opaques;
		std::vector<GameObject*> m_transparents;
		std::vector<GameObject*> m_gizmos;
	};
}