#pragma once
#include <wrl/client.h>
#include <memory>
#include <array>
#include <d3d11.h>

#include "Graphics/ConstantBuffer.h"
#include "Graphics/ConstantBufferDatas.h"

namespace Dive
{
	class Graphics;
	class RenderTexture;
	class RenderTexture;

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

		enum class eConstantBuffer : uint8_t
		{	
			Frame,
			Material,
			Object,
			Light,
			Count
		};

	public:
		explicit Renderer(Graphics* graphics, uint32_t width, uint32_t height);
		~Renderer();

		void Update(float dt);
		void Render();

		void ResolveToOffScreenTexture();
		void ResolveToBackbuffer();

		void OnResize(uint32_t width, uint32_t height);

		const RenderTexture* GetOffScreenTexture() const { return m_offScreenRenderTarget.get(); }

	private:
		void createDepthStencilStates();
		void createRasterizerStates();
		void createBlendStates();
		void createSamplers();
		void createCBuffers();
		void createResolutionDependantResources(uint32_t width, uint32_t height);

		void createRenderTargets(uint32_t width, uint32_t height);
		void createGBuffer(uint32_t width, uint32_t height);
		void createRenderPasses(uint32_t width, uint32_t height);

		void loadTextures();

		void bindGlobals();

		void passTest();
		void passGBuffer();
		void passPicking();
		void passAmbient();
		void passDeferredLighting();
		void passForward();
		void passSkybox();

	private:
		Graphics* m_graphics = nullptr;
		uint32_t m_width;
		uint32_t m_height;

		std::array<Microsoft::WRL::ComPtr<ID3D11DepthStencilState>, static_cast<size_t>(eDepthStencilState::Count)> m_depthStencilStates;
		std::array<Microsoft::WRL::ComPtr<ID3D11RasterizerState>, static_cast<size_t>(eRasterizerState::Count)>m_rasterizerStates;
		std::array<Microsoft::WRL::ComPtr<ID3D11BlendState>, static_cast<size_t>(eBlendState::Count)> m_blendStates;
		std::array<Microsoft::WRL::ComPtr<ID3D11SamplerState>, static_cast<size_t>(eSamplerState::Count)> m_samplerStates;
		
		// Graphics에 GfxConstantBuffer, Renderer에 ConstantBuffer로 나누어 관리했다.

		// render targets
		std::unique_ptr<RenderTexture> m_ldrRenderTarget;
		std::unique_ptr<RenderTexture> m_offScreenRenderTarget;

		FrameData m_frameData;
		std::unique_ptr<ConstantBuffer<FrameData>> m_cbFrame;
		MaterialData m_materialData;
		std::unique_ptr<ConstantBuffer<MaterialData>> m_cbMaterial;
		ObjectData m_objectData;
		std::unique_ptr<ConstantBuffer<ObjectData>> m_cbObject;
		LightData m_lightData;
		std::unique_ptr<ConstantBuffer<LightData>> m_cbLight;
	};
}