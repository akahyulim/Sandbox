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

	enum class eSkyMode : uint8_t
	{
		Skybox,
		ClearColor
	};

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
			AlphaEnabled,
			AlphaDisabled,
			Additive,
			Count
		};

		enum class eSamplerState : uint8_t
		{
			WrapLinear,
			ClampPoint,
			ClampLinear,
			Skybox,
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

		enum class eConstantBuffer : uint8_t
		{	
			Frame,
			Object,
			Material,
			Weather,
			Light,
			SelectedObject,
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

		eSkyMode GetSkyMode() const { return m_skyMode; }
		void SetSkyMode(eSkyMode mode) { m_skyMode = mode; }

		DirectX::XMFLOAT4 GetSkyColor() const { return m_skyColor; }
		void SetSkyColor(const DirectX::XMFLOAT4& color) { m_skyColor = color; }
		void SetSkyColor(float r, float g, float b, float a) { m_skyColor = { r, g, b, a }; }

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
		
		void updateWeather();

		void passGBuffer(Scene* scene);
		void passDeferredLighting();
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

		FrameData m_frameData{};
		std::unique_ptr<ConstantBuffer<FrameData>> m_cbFrame;
		ObjectData m_objectData{};
		std::unique_ptr<ConstantBuffer<ObjectData>> m_cbObject;
		MaterialData m_materialData{};
		std::unique_ptr<ConstantBuffer<MaterialData>> m_cbMaterial;
		WeatherData m_weatherData{};
		std::unique_ptr<ConstantBuffer<WeatherData>> m_cbWeather;
		LightData m_lightData{};
		std::unique_ptr<ConstantBuffer<LightData>> m_cbLight;

		uint32_t m_lastSelectedObjectID = 0;
		std::unique_ptr<ConstantBuffer<SelectedObjectData>> m_cbSelectedObject;

		PickingData m_pickingData{};
		std::unique_ptr<StructuredBuffer<PickingData>> m_pickingBuffer;


		RenderPassDesc m_gbufferPass;
		RenderPassDesc m_deferredLightingPass;
		RenderPassDesc m_offScreenResolvePass;
		RenderPassDesc m_skyboxPass;	// 원래는 forward를 사용

		std::unique_ptr<VertexBuffer> m_cubeVB;
		std::unique_ptr<IndexBuffer> m_cubeIB;

		DirectX::XMFLOAT2 m_mousePosition;

		DirectX::XMFLOAT4 m_skyColor = { 0.0f, 0.75f, 1.0f , 0.0f };
		eSkyMode m_skyMode = eSkyMode::Skybox;
	};
}