#include "pch.h"
#include "Renderer.h"
#include "TextureManager.h"
#include "Material.h"
#include "ShaderManager.h"
#include "Graphics/Graphics.h"
#include "Graphics/ConstantBufferDatas.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/RenderTexture.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/Geometry.h"

#include "Scene/Scene.h"
#include "Scene/GameObject.h"
#include "Scene/Components/Transform.h"
#include "Scene/Components/Camera.h"
#include "Scene/Components/MeshRenderer.h"

namespace Dive
{
	Renderer::Renderer(Graphics* graphics, uint32_t width, uint32_t height)
		: m_graphics(graphics)
		, m_width(width)
		, m_height(height)
        , m_mousePosition{0.0f, 0.0f}

	{
		createDepthStencilStates();
        createRasterizerStates();
        createBlendStates();
		createSamplers();
        createBuffers();
        loadTextures();

		createResolutionDependantResources(width, height);
	}

	Renderer::~Renderer() = default;
	
    // adria의 Tick과 같다.
    // 다른점은 Camera가 아니라 Scene을 받아온다는 점
    // 실제로 Scene은 Camera를 가져오는 역할만 한다.
    // adria는 dt를 전달받는 Update가 별도로 존재하고
    // 그 곳에선 데이터들을 갱신한다.
    // 그런데 나의 경우엔 Scene - GameObject - Component의 업데이트가 존재한다.
	void Renderer::Update(Scene* scene)
	{
        if (!scene)
            return;

        bindGlobals();

        auto camera = scene->GetCamera()->GetComponent<Camera>();

        camera->SetAspectRatio(static_cast<float>(m_width), static_cast<float>(m_height));

        m_frameData.cameraPosition = DirectX::XMFLOAT4(
            camera->GetTransform()->GetPosition().x,
            camera->GetTransform()->GetPosition().y,
            camera->GetTransform()->GetPosition().z,
            1.0f);
        m_frameData.cameraForward = DirectX::XMFLOAT4(
            camera->GetTransform()->GetForward().x,
            camera->GetTransform()->GetForward().y,
            camera->GetTransform()->GetForward().z,
            1.0f);
        m_frameData.view = DirectX::XMMatrixTranspose(camera->GetViewMatrix());
        m_frameData.projection = DirectX::XMMatrixTranspose(camera->GetProjectionMatrix());
        m_frameData.viewProjection = DirectX::XMMatrixTranspose(camera->GetViewProjMatrix());
        m_frameData.inverseViewProjection = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, camera->GetViewProjMatrix()));
        m_frameData.screenResolution.x = static_cast<float>(m_width);
        m_frameData.screenResolution.y = static_cast<float>(m_height);
        m_frameData.mousePosition = m_mousePosition;
        m_cbFrame->Update(m_graphics, m_frameData);
	}
	
    // Render에는 RenderSettings가 전달
    // Editor의 멤버 변수이며 Editor::Run -> Engine::Run -> Render -> Renderer::Render 순으로 전달
    // 각종 렌더링 옵션들로 구성되어 있으며 Editor에서 취사 선택이 가능한 형태
    // 게임에서 그래픽스 옵션이라고 볼 수 있다.
	void Renderer::Render(Scene* scene)
    {	
        if (!scene)
            return;

        passGBuffer(scene);
        passPicking();
        passDeferredLighting();
        passSkybox(scene);
        //passForward();
        //passPostProcessing();
	}

    void Renderer::ResolveToOffScreenTexture()
    {
        m_graphics->BeginRenderPass(m_offScreenResolvePass);

        m_graphics->SetViewport(m_offScreenRenderTarget->GetWidth(), m_offScreenRenderTarget->GetHeight());
        auto srv = m_ldrRenderTarget->GetShaderResourceView();
        m_graphics->SetShaderResourceView(eShaderStage::PS, 12, &srv);  // 원래 srv slot enum class가 없었나...
        ShaderManager::Get().GetShaderProgram(eShaderPrograms::Resolve)->Bind(m_graphics);
        m_graphics->SetTopology(ePrimitiveTopology::TriangleStrip);
        m_graphics->SetVertexBuffer(nullptr);
        m_graphics->Draw(4);

        m_graphics->EndRenderPass();
    }

    void Renderer::ResolveToBackbuffer()
    {
        m_graphics->SetBackbuffer();

        auto srv = m_ldrRenderTarget->GetShaderResourceView();
        m_graphics->SetShaderResourceView(eShaderStage::PS, 12, &srv);  // 원래 srv slot enum class가 없었나...
        ShaderManager::Get().GetShaderProgram(eShaderPrograms::Resolve)->Bind(m_graphics);
        m_graphics->SetTopology(ePrimitiveTopology::TriangleStrip);
        m_graphics->SetVertexBuffer(nullptr);
        m_graphics->Draw(4);
    }

    void Renderer::OnResize(uint32_t width, uint32_t height)
    {
        m_width = width;
        m_height = height;

        if (m_width != 0 || m_height != 0)
        {
            createResolutionDependantResources(width, height);
        }
    }

    void Renderer::SetMousePosition(const DirectX::XMUINT2& cursorPos)
    {
        m_mousePosition.x = static_cast<float>(cursorPos.x);
        m_mousePosition.y = static_cast<float>(cursorPos.y);
    }

    void Renderer::createDepthStencilStates()
    {
        D3D11_DEPTH_STENCIL_DESC desc{};
        D3D11_DEPTH_STENCILOP_DESC stencilOp{};

        // 1. Default (기본 뎁스 읽기/쓰기)
        ZeroMemory(&desc, sizeof(desc));
        desc.DepthEnable = true;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        desc.DepthFunc = D3D11_COMPARISON_LESS;
        desc.StencilEnable = false;
        desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
        desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
        stencilOp = { D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS };
        desc.FrontFace = stencilOp;
        desc.BackFace = stencilOp;

        m_depthStencilStates[static_cast<size_t>(eDepthStencilState::Default)] =
            m_graphics->CreateDepthStencilState(desc);

        // 2. StencilMark (스텐실 기록용)
        ZeroMemory(&desc, sizeof(desc));
        desc.DepthEnable = true;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        desc.DepthFunc = D3D11_COMPARISON_LESS;
        desc.StencilEnable = true;
        desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
        desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
        stencilOp = { D3D11_STENCIL_OP_REPLACE, D3D11_STENCIL_OP_REPLACE, D3D11_STENCIL_OP_REPLACE, D3D11_COMPARISON_ALWAYS };
        desc.FrontFace = stencilOp;
        desc.BackFace = stencilOp;

        m_depthStencilStates[static_cast<size_t>(eDepthStencilState::StencilMark)] =
            m_graphics->CreateDepthStencilState(desc);

        // 3. GBuffer
        ZeroMemory(&desc, sizeof(desc));
        desc.DepthEnable = true;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        desc.DepthFunc = D3D11_COMPARISON_LESS;
        desc.StencilEnable = true;
        desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
        desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
        stencilOp = { D3D11_STENCIL_OP_REPLACE, D3D11_STENCIL_OP_REPLACE, D3D11_STENCIL_OP_REPLACE, D3D11_COMPARISON_ALWAYS };
        desc.FrontFace = stencilOp;
        desc.BackFace = stencilOp;

        m_depthStencilStates[static_cast<size_t>(eDepthStencilState::GBuffer)] =
            m_graphics->CreateDepthStencilState(desc);

        // 4. DepthDisabled
        ZeroMemory(&desc, sizeof(desc));
        desc.DepthEnable = false;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        desc.DepthFunc = D3D11_COMPARISON_LESS;
        desc.StencilEnable = true;
        desc.StencilReadMask = 0xFF;
        desc.StencilWriteMask = 0xFF;
        desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
        desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
        desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        m_depthStencilStates[static_cast<size_t>(eDepthStencilState::DepthDisabled)] =
            m_graphics->CreateDepthStencilState(desc);

        // 5. ForwardLight
        ZeroMemory(&desc, sizeof(desc));
        desc.DepthEnable = true;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        desc.StencilEnable = false;
        desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
        desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
        const D3D11_DEPTH_STENCILOP_DESC noSkyStencilOp = { D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_EQUAL };
        desc.FrontFace = noSkyStencilOp;
        desc.BackFace = noSkyStencilOp;

        m_depthStencilStates[static_cast<size_t>(eDepthStencilState::ForwardLight)] =
            m_graphics->CreateDepthStencilState(desc);

        // 6. Transparent
        ZeroMemory(&desc, sizeof(desc));
        desc.DepthEnable = true;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        desc.DepthFunc = D3D11_COMPARISON_LESS;
        desc.StencilEnable = false;

        m_depthStencilStates[static_cast<size_t>(eDepthStencilState::Transparent)] =
            m_graphics->CreateDepthStencilState(desc);

        // 7. Skybox
        ZeroMemory(&desc, sizeof(desc));
        desc.DepthEnable = true;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;// D3D11_COMPARISON_ALWAYS;
        desc.StencilEnable = false;

        m_depthStencilStates[static_cast<size_t>(eDepthStencilState::Skybox)] =
            m_graphics->CreateDepthStencilState(desc);
    }

    void Renderer::createRasterizerStates()
    {
        D3D11_RASTERIZER_DESC desc{};

        // 공통 기본값 설정
        desc.DepthBias = 0;
        desc.DepthBiasClamp = 0.0f;
        desc.SlopeScaledDepthBias = 0.0f;
        desc.DepthClipEnable = true;
        desc.ScissorEnable = false;
        desc.MultisampleEnable = false;
        desc.AntialiasedLineEnable = false;

        // 1. FillSolid_CullFront
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_FRONT;
        desc.FrontCounterClockwise = true;

        m_rasterizerStates[static_cast<size_t>(eRasterizerState::FillSolid_CullFront)] =
            m_graphics->CreateRasterizerState(desc);

        // 2. FillSolid_CullBack
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_BACK;
        desc.FrontCounterClockwise = false;

        m_rasterizerStates[static_cast<size_t>(eRasterizerState::FillSolid_CullBack)] =
            m_graphics->CreateRasterizerState(desc);

        // 3. FillSolid_CullNone
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_NONE;
        desc.FrontCounterClockwise = false; // 필요에 따라 설정

        m_rasterizerStates[static_cast<size_t>(eRasterizerState::FillSolid_CullNone)] =
            m_graphics->CreateRasterizerState(desc);
    }

    void Renderer::createBlendStates()
    {
        D3D11_BLEND_DESC desc{};
        desc.AlphaToCoverageEnable = false;
        desc.IndependentBlendEnable = false;

        // AlphaEnabled
        {
            const D3D11_RENDER_TARGET_BLEND_DESC alphaBlendDesc =
            {
                true,
                D3D11_BLEND_SRC_ALPHA,
                D3D11_BLEND_INV_SRC_ALPHA,
                D3D11_BLEND_OP_ADD,
                D3D11_BLEND_ONE,
                D3D11_BLEND_INV_SRC_ALPHA,
                D3D11_BLEND_OP_ADD,
                D3D11_COLOR_WRITE_ENABLE_ALL
            };

            for (UINT i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
                desc.RenderTarget[i] = alphaBlendDesc;

            m_blendStates[static_cast<size_t>(eBlendState::AlphaEnabled)] =
                m_graphics->CreateBlendState(desc);
        }

        // AlphaDisabled
        {
            const D3D11_RENDER_TARGET_BLEND_DESC alphaDisabledDesc =
            {
                false,
                D3D11_BLEND_SRC_ALPHA,
                D3D11_BLEND_INV_SRC_ALPHA,
                D3D11_BLEND_OP_ADD,
                D3D11_BLEND_ONE,
                D3D11_BLEND_INV_SRC_ALPHA,
                D3D11_BLEND_OP_ADD,
                D3D11_COLOR_WRITE_ENABLE_ALL
            };

            for (UINT i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
                desc.RenderTarget[i] = alphaDisabledDesc;

            m_blendStates[static_cast<size_t>(eBlendState::AlphaDisabled)] =
                m_graphics->CreateBlendState(desc);
        }

        // Additive
        {
            const D3D11_RENDER_TARGET_BLEND_DESC additiveBlendDesc =
            {
                true,
                D3D11_BLEND_ONE,
                D3D11_BLEND_ONE,
                D3D11_BLEND_OP_ADD,
                D3D11_BLEND_ONE,
                D3D11_BLEND_ONE,
                D3D11_BLEND_OP_ADD,
                D3D11_COLOR_WRITE_ENABLE_ALL,
            };

            for (UINT i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
                desc.RenderTarget[i] = additiveBlendDesc;

            m_blendStates[static_cast<size_t>(eBlendState::Additive)] =
                m_graphics->CreateBlendState(desc);
        }
    }

    void Renderer::createSamplers()
    {
        D3D11_SAMPLER_DESC desc{};

        // WrapLinear
        {
            desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
            desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
            desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
            desc.MipLODBias = 0.0f;
            desc.MaxAnisotropy = 1;
            desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
            desc.BorderColor[0] = 0;
            desc.BorderColor[1] = 0;
            desc.BorderColor[2] = 0;
            desc.BorderColor[3] = 0;
            desc.MinLOD = 0;
            desc.MaxLOD = D3D11_FLOAT32_MAX;

            m_samplerStates[static_cast<size_t>(eSamplerState::WrapLinear)] =
                m_graphics->CreateSamplerState(desc);
        }

        // ClampPoint
        {
            ZeroMemory(&desc, sizeof(desc));
            desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
            desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
            desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
            desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
            desc.MinLOD = 0;
            desc.MaxLOD = D3D11_FLOAT32_MAX;

            m_samplerStates[static_cast<size_t>(eSamplerState::ClampPoint)] =
                m_graphics->CreateSamplerState(desc);
        }

        // ClampLinear
        {
            ZeroMemory(&desc, sizeof(desc));
            desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
            desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
            desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            desc.MaxAnisotropy = 1;
            desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
            desc.MinLOD = 0;
            desc.MaxLOD = D3D11_FLOAT32_MAX;

            m_samplerStates[static_cast<size_t>(eSamplerState::ClampLinear)] =
                m_graphics->CreateSamplerState(desc);
        }

        // Skybox
        {
            ZeroMemory(&desc, sizeof(desc));
            desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
            desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
            desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            desc.MaxAnisotropy = 1;
            desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
            desc.MinLOD = 0;
            desc.MaxLOD = D3D11_FLOAT32_MAX;

            m_samplerStates[static_cast<size_t>(eSamplerState::Skybox)] =
                m_graphics->CreateSamplerState(desc);
        }

        // ShadowCompare
        {
            ZeroMemory(&desc, sizeof(desc));
            desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
            desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
            desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
            desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
            desc.ComparisonFunc = D3D11_COMPARISON_LESS;
            desc.BorderColor[0] = 1.0f;
            desc.BorderColor[1] = 1.0f;
            desc.BorderColor[2] = 1.0f;
            desc.BorderColor[3] = 1.0f;
            desc.MinLOD = 0.0f;
            desc.MaxLOD = D3D11_FLOAT32_MAX;
            desc.MipLODBias = 0.0f;
            desc.MaxAnisotropy = 1;

            m_samplerStates[static_cast<size_t>(eSamplerState::ShadowCompare)] =
                m_graphics->CreateSamplerState(desc);
        }
    }

    void Renderer::createBuffers()
    {
        m_cbFrame = std::make_unique<ConstantBuffer<FrameData>>(m_graphics); 
        m_cbObject = std::make_unique<ConstantBuffer<ObjectData>>(m_graphics);
        m_cbMaterial = std::make_unique<ConstantBuffer<MaterialData>>(m_graphics);
        m_cbLight = std::make_unique<ConstantBuffer<LightData>>(m_graphics);

        m_pickingBuffer = std::make_unique<StructuredBuffer<PickingData>>(m_graphics, eStructuredBufferType::Read);

        const SimpleVertex vertices[] = 
		{
			DirectX::XMFLOAT3{ -0.5f, -0.5f,  0.5f },
			DirectX::XMFLOAT3{  0.5f, -0.5f,  0.5f },
			DirectX::XMFLOAT3{  0.5f,  0.5f,  0.5f },
			DirectX::XMFLOAT3{ -0.5f,  0.5f,  0.5f },
			DirectX::XMFLOAT3{ -0.5f, -0.5f, -0.5f },
			DirectX::XMFLOAT3{  0.5f, -0.5f, -0.5f },
			DirectX::XMFLOAT3{  0.5f,  0.5f, -0.5f },
			DirectX::XMFLOAT3{ -0.5f,  0.5f, -0.5f }
		};

		const uint16_t indices[] = 
		{
			0, 1, 2,
			2, 3, 0,
			1, 5, 6,
			6, 2, 1,
			7, 6, 5,
			5, 4, 7,
			4, 0, 3,
			3, 7, 4,
			4, 5, 1,
			1, 0, 4,
			3, 2, 6,
			6, 7, 3
		};

        m_cubeVB = std::make_unique<VertexBuffer>(m_graphics, (uint32_t)sizeof(SimpleVertex), 8, vertices);
        m_cubeIB = std::make_unique<IndexBuffer>(m_graphics, eFormat::R16_UINT, 36, indices);
    }

    void Renderer::createResolutionDependantResources(uint32_t width, uint32_t height)
    {
        // 여기에서 Camera의 AspectRatio를 설정하는 게 제격인데
        // Scene을 끌고 오기가 싫다.
        // adria는 Camera를 멤버 변수로 두고 Tick에서 받아와 저장한다.

        createRenderTargets(width, height);
        createGBuffer(width, height);
        createRenderPasses(width, height);
    }

    void Renderer::createRenderTargets(uint32_t width, uint32_t height)
    {
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = width;
        desc.Height = height;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET; 
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.MiscFlags = 0;

        //desc.Format = DXGI_FORMAT_R32_UINT;
        //m_objectIDRenderTarget = std::make_unique<RenderTexture>(m_graphics, desc);

        desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        m_hdrRenderTarget = std::make_unique<RenderTexture>(m_graphics, desc);

        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        m_ldrRenderTarget = std::make_unique<RenderTexture>(m_graphics, desc);
        m_offScreenRenderTarget = std::make_unique<RenderTexture>(m_graphics, desc);

        desc.Format = DXGI_FORMAT_R16_TYPELESS;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
        m_depthTarget = std::make_unique<RenderTexture>(m_graphics, desc);
    }

    void Renderer::createGBuffer(uint32_t width, uint32_t height)
    {
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = width;
        desc.Height = height;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.MiscFlags = 0;

        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        m_gbuffer[static_cast<size_t>(eGBufferType::AlbedoRoughness)] = std::make_unique<RenderTexture>(m_graphics, desc);

        desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        m_gbuffer[static_cast<size_t>(eGBufferType::NormalMetallic)] = std::make_unique<RenderTexture>(m_graphics, desc);

        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        m_gbuffer[static_cast<size_t>(eGBufferType::Emissive)] = std::make_unique<RenderTexture>(m_graphics, desc);

        desc.Format = DXGI_FORMAT_R32_UINT;
        m_gbuffer[static_cast<size_t>(eGBufferType::ObjectID)] = std::make_unique<RenderTexture>(m_graphics, desc);

        desc.Format = DXGI_FORMAT_R32_TYPELESS;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
        m_gbuffer[static_cast<size_t>(eGBufferType::Depth)] = std::make_unique<RenderTexture>(m_graphics, desc);
    }

    void Renderer::createRenderPasses(uint32_t width, uint32_t height)
    {
        // gbuffer 
        {
            RenderTargetDesc albedoDesc;
            albedoDesc.RenderTargetView = m_gbuffer[static_cast<size_t>(eGBufferType::AlbedoRoughness)]->GetRenderTargetView();
            albedoDesc.ClearColor[0] = 0.0f;
            albedoDesc.ClearColor[1] = 0.0f;
            albedoDesc.ClearColor[2] = 0.0f;
            albedoDesc.ClearColor[3] = 0.0f;
            albedoDesc.AccessType = eLoadAccessOp::Clear;

            RenderTargetDesc normalDesc;
            normalDesc.RenderTargetView = m_gbuffer[static_cast<size_t>(eGBufferType::NormalMetallic)]->GetRenderTargetView();
            normalDesc.ClearColor[0] = 0.0f;
            normalDesc.ClearColor[1] = 0.0f;
            normalDesc.ClearColor[2] = 0.0f;
            normalDesc.ClearColor[3] = 0.0f;
            normalDesc.AccessType = eLoadAccessOp::Clear;

            RenderTargetDesc emissiveDesc;
            emissiveDesc.RenderTargetView = m_gbuffer[static_cast<size_t>(eGBufferType::Emissive)]->GetRenderTargetView();
            emissiveDesc.ClearColor[0] = 0.0f;
            emissiveDesc.ClearColor[1] = 0.0f;
            emissiveDesc.ClearColor[2] = 0.0f;
            emissiveDesc.ClearColor[3] = 0.0f;
            emissiveDesc.AccessType = eLoadAccessOp::Clear;

            RenderTargetDesc objectIDDesc;
            objectIDDesc.RenderTargetView = m_gbuffer[static_cast<size_t>(eGBufferType::ObjectID)]->GetRenderTargetView();
            objectIDDesc.ClearColor[0] = 0.0f;
            objectIDDesc.ClearColor[1] = 0.0f;
            objectIDDesc.ClearColor[2] = 0.0f;
            objectIDDesc.ClearColor[3] = 0.0f;
            objectIDDesc.AccessType = eLoadAccessOp::Clear;

            DepthStencilDesc dsDesc;
            dsDesc.DepthStencilView = m_gbuffer[static_cast<size_t>(eGBufferType::Depth)]->GetDetphStencilView();
            dsDesc.ClearFlags = D3D11_CLEAR_DEPTH;
            dsDesc.AccessType = eLoadAccessOp::Clear;

            RenderPassDesc desc{};
            desc.renderTargetDescs.push_back(albedoDesc);
            desc.renderTargetDescs.push_back(normalDesc);
            desc.renderTargetDescs.push_back(emissiveDesc);
            desc.renderTargetDescs.push_back(objectIDDesc);
            desc.depthStencilDesc = dsDesc;
            m_gbufferPass = desc;
        }

        // Deferred Lighting
        {
            RenderTargetDesc rtDesc;
            rtDesc.RenderTargetView = m_ldrRenderTarget->GetRenderTargetView();
            rtDesc.ClearColor[0] = 0.0f;
            rtDesc.ClearColor[1] = 0.0f;
            rtDesc.ClearColor[2] = 0.0f;
            rtDesc.ClearColor[3] = 0.0f;
            rtDesc.AccessType = eLoadAccessOp::Clear;

            RenderPassDesc desc{};
            desc.renderTargetDescs.push_back(rtDesc);
            m_deferredLightingPass = desc;
        }

        // Skybox
        {
            RenderTargetDesc rtDesc;
            rtDesc.RenderTargetView = m_ldrRenderTarget->GetRenderTargetView();
            rtDesc.ClearColor[0] = 0.0f;
            rtDesc.ClearColor[1] = 0.0f;
            rtDesc.ClearColor[2] = 0.0f;
            rtDesc.ClearColor[3] = 0.0f;
            rtDesc.AccessType = eLoadAccessOp::Load;

            DepthStencilDesc dsDesc;
            dsDesc.DepthStencilView = m_gbuffer[static_cast<size_t>(eGBufferType::Depth)]->GetDetphStencilView();
            dsDesc.AccessType = eLoadAccessOp::Load;

            RenderPassDesc desc{};
            desc.renderTargetDescs.push_back(rtDesc);
            desc.depthStencilDesc = dsDesc;
            m_skyboxPass = desc;
        }

        // offScreen Resolve
        {
            RenderTargetDesc rtDesc;
            rtDesc.AccessType = eLoadAccessOp::Clear;
            rtDesc.RenderTargetView = m_offScreenRenderTarget->GetRenderTargetView();
            rtDesc.ClearColor[0] = 0.0f;
            rtDesc.ClearColor[1] = 0.0f;
            rtDesc.ClearColor[2] = 0.0f;
            rtDesc.ClearColor[3] = 0.0f;

            RenderPassDesc desc{};
            desc.renderTargetDescs.push_back(rtDesc);
            m_offScreenResolvePass = desc;
        }
    }

    void Renderer::loadTextures()
    {
    }

    void Renderer::bindGlobals()
    {
        static bool called = false;

        if (!called)
        {
            auto deviceContext = m_graphics->GetDeviceContext();

            // vs
            m_cbFrame->Bind(m_graphics, eShaderStage::VS, static_cast<uint32_t>(eConstantBuffer::Frame));
            m_cbObject->Bind(m_graphics, eShaderStage::VS, static_cast<uint32_t>(eConstantBuffer::Object));
            
            // ps
            m_cbFrame->Bind(m_graphics, eShaderStage::PS, static_cast<uint32_t>(eConstantBuffer::Frame));
            m_cbObject->Bind(m_graphics, eShaderStage::PS, static_cast<uint32_t>(eConstantBuffer::Object));
            m_cbMaterial->Bind(m_graphics, eShaderStage::PS, static_cast<uint32_t>(eConstantBuffer::Material));
            //m_cbLight->Bind(m_graphics, eShaderStage::PS, static_cast<uint32_t>(eConstantBuffer::Light));

            // cs
            m_cbFrame->Bind(m_graphics, eShaderStage::CS, static_cast<uint32_t>(eConstantBuffer::Frame));

            ID3D11SamplerState* samplers[static_cast<size_t>(eSamplerState::Count)] =
            {
                m_samplerStates[static_cast<size_t>(eSamplerState::WrapLinear)].Get(),
                m_samplerStates[static_cast<size_t>(eSamplerState::ClampPoint)].Get(),
                m_samplerStates[static_cast<size_t>(eSamplerState::ClampLinear)].Get(),
                m_samplerStates[static_cast<size_t>(eSamplerState::Skybox)].Get(),
                m_samplerStates[static_cast<size_t>(eSamplerState::ShadowCompare)].Get()
            };
            deviceContext->PSSetSamplers(0, static_cast<UINT>(eSamplerState::Count), samplers);

            called = true;
        }
    }

    void Renderer::passGBuffer(Scene* scene)
    {
        m_graphics->BeginRenderPass(m_gbufferPass);

        m_graphics->SetViewport(m_ldrRenderTarget->GetWidth(), m_ldrRenderTarget->GetHeight());

        m_graphics->SetRasterizerState(m_rasterizerStates[(size_t)eRasterizerState::FillSolid_CullBack].Get());
        m_graphics->SetDepthStencilState(m_depthStencilStates[(size_t)eDepthStencilState::Default].Get(), 0);

        ShaderManager::Get().GetShaderProgram(eShaderPrograms::GBuffer)->Bind(m_graphics);

        for (auto renderable : scene->GetRenderables())
        {
            auto transform = renderable->GetTransform();
            auto staticMesh = renderable->GetComponent<MeshRenderer>();
            auto material = staticMesh->GetMaterial();

            m_objectData.model = DirectX::XMMatrixTranspose(transform->GetWorldMatrix());
            m_objectData.id = staticMesh->GetObjectID();
            m_cbObject->Update(m_graphics, m_objectData);

            m_materialData.baseColor = material->GetBaseColor();
            m_materialData.offset = material->GetOffset();
            m_materialData.tiling = material->GetTiling();
            m_materialData.flags = material->GetFlags();
            m_cbMaterial->Update(m_graphics, m_materialData);

            staticMesh->Draw(m_graphics);
        }

        m_graphics->EndRenderPass();
    }

    void Renderer::passPicking()
    {
        // gbuffer 중 normal, depth의 srv 사용
        auto normalSrv = m_gbuffer[static_cast<size_t>(eGBufferType::NormalMetallic)]->GetShaderResourceView();
        auto idSrv = m_gbuffer[static_cast<size_t>(eGBufferType::ObjectID)]->GetShaderResourceView();
        auto depthSrv = m_gbuffer[static_cast<size_t>(eGBufferType::Depth)]->GetShaderResourceView();
        m_graphics->SetShaderResourceView(eShaderStage::CS, 7, &normalSrv);
        m_graphics->SetShaderResourceView(eShaderStage::CS, 9, &idSrv);
        m_graphics->SetShaderResourceView(eShaderStage::CS, 10, &depthSrv);

        // uav도 사용
        auto uav = m_pickingBuffer->GetUAV();
        m_graphics->GetDeviceContext()->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);

        ShaderManager::Get().GetShaderProgram(eShaderPrograms::Picking)->Bind(m_graphics);
        m_graphics->GetDeviceContext()->Dispatch(1, 1, 1);      // 추후 랩핑 필요

        m_graphics->SetShaderResourceView(eShaderStage::CS, 7, nullptr);
        m_graphics->SetShaderResourceView(eShaderStage::CS, 9, nullptr);
        m_graphics->SetShaderResourceView(eShaderStage::CS, 10, nullptr);
        ID3D11UnorderedAccessView* nullUAV = nullptr;
        m_graphics->GetDeviceContext()->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
        // 다른 cs를 바인딩하지 않으므로 데이터 갱신을 위해 명시적으로 해제
        ShaderManager::Get().GetShaderProgram(eShaderPrograms::Picking)->Unbind(m_graphics);
        
        // uav를 map/unmap 해서 계산 결과를 복사
        D3D11_MAPPED_SUBRESOURCE mapped_buffer{};
        auto hr = m_graphics->GetDeviceContext()->Map((ID3D11Resource*)m_pickingBuffer->GetBuffer(), 0u, D3D11_MAP_READ, 0u, &mapped_buffer);
        if (FAILED(hr))
        {
            spdlog::error("StructuredBuffer Map 실패: {}", ErrorUtils::ToVerbose(hr));
            return;
        }
        const PickingData* data = (const PickingData*)mapped_buffer.pData;
        m_pickingData = *data;
        m_graphics->GetDeviceContext()->Unmap(m_pickingBuffer->GetBuffer(), 0);
    }
    
    void Renderer::passDeferredLighting()
    {
        m_graphics->BeginRenderPass(m_deferredLightingPass);

        m_graphics->SetViewport(m_ldrRenderTarget->GetWidth(), m_ldrRenderTarget->GetHeight());

        m_graphics->SetRasterizerState(m_rasterizerStates[(size_t)eRasterizerState::FillSolid_CullNone].Get());
        m_graphics->SetDepthStencilState(nullptr, 0);
        m_graphics->SetTopology(ePrimitiveTopology::TriangleStrip);

        ShaderManager::Get().GetShaderProgram(eShaderPrograms::DeferredLighting)->Bind(m_graphics);

        // SetGBufferSRV 같은 걸 만드는 게 나을 듯하다.
        auto srv = m_gbuffer[0]->GetShaderResourceView();
        m_graphics->SetShaderResourceView(eShaderStage::PS, 6, &srv);

        m_graphics->SetVertexBuffer(nullptr);
        m_graphics->Draw(4);

        m_graphics->EndRenderPass();
    }

    void Renderer::passSkybox(Scene* scene)
    {
        // adria에선 passForward안에서 다수의 pass가 호출되며 이때 m_forwardPass를 사용한다.
        m_graphics->BeginRenderPass(m_skyboxPass);

        m_graphics->SetViewport(m_ldrRenderTarget->GetWidth(), m_ldrRenderTarget->GetHeight());

        m_graphics->SetRasterizerState(m_rasterizerStates[(size_t)eRasterizerState::FillSolid_CullNone].Get());
        m_graphics->SetDepthStencilState(m_depthStencilStates[(size_t)eDepthStencilState::Skybox].Get(), 0);

        ShaderManager::Get().GetShaderProgram(eShaderPrograms::Skybox)->Bind(m_graphics);
        
        auto& envData = scene->GetEnviroment();
        auto srv = TextureManager::Get().GetTextureView(envData.skyboxCubemap);
        m_graphics->SetShaderResourceView(eShaderStage::PS, 11, &srv);

        auto camera = scene->GetCamera();
        m_objectData.model = DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslationFromVector(camera->GetTransform()->GetPositionVector()));
        m_cbObject->Update(m_graphics, m_objectData);

        m_graphics->SetTopology(ePrimitiveTopology::TriangleList);
        
        m_graphics->SetVertexBuffer(m_cubeVB.get());
        m_graphics->SetIndexBuffer(m_cubeIB.get());
        m_graphics->DrawIndexed(m_cubeIB->GetCount());
        
        m_graphics->SetRasterizerState(nullptr);
        m_graphics->SetDepthStencilState(nullptr, 0);

        m_graphics->EndRenderPass();
    }

    void Renderer::passForward()
    {
        // 반투명
        // 지버퍼에서 objectID만 가져와 그려야 한다.
        // 이때 clear가 아니라 Load로 설정해야 한다.
        // 좀 더 명확하게 하자면 Load도 아니고 Add를 추가해야 한다.
    }

    void Renderer::passPostProcessing()
    {
        // ldr
    }

    /*
    // DX12의 기초적인 Pass 구조
    {
        // 1. 렌더 패스 시작 (RTV, DSV 및 Load/Store 정책 선언)
        commandList->BeginRenderPass(1, &rtDesc, &dsDesc, ...);

        // 2. 뷰포트 및 시저 렉트 설정 (화면 어디에 그릴 것인가)
        commandList->RSSetViewports(1, &viewport);
        commandList->RSSetScissorRects(1, &scissorRect);

        // 3. 파이프라인 상태(PSO) 및 루트 시그니처 바인딩 (이 패스의 '전역 스타일' 결정)
        // PSO에서 Shader까지 바인딩한다고...
        commandList->SetPipelineState(pipelineState.Get());
        commandList->SetGraphicsRootSignature(rootSignature.Get());

        // 4. 프리미티브 토폴로지 설정 (삼각형 리스트 등)
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // ==========================================
        // 5. 개별 객체(Renderable) 단위의 바인딩 및 드로우
        // ==========================================
        for (auto& object : renderables)
        {
            // 5-1. 버퍼 바인딩 (VertexBuffer, IndexBuffer)
            commandList->IASetVertexBuffers(0, 1, &object->GetVBView());
            commandList->IASetIndexBuffer(&object->GetIBView());

            // 5-2. 상수 버퍼(CBV)나 텍스처(SRV) 바인딩 (Root Signature를 통해 전달)
            commandList->SetGraphicsRootConstantBufferView(0, object->GetConstantBufferGPUAddress());
            // 혹은 Descriptor Table 바인딩...

            // 5-3. 실제 드로우 콜 수행!
            commandList->DrawIndexedInstanced(object->GetIndexCount(), 1, 0, 0, 0);
        }

        // 6. 렌더 패스 종료
        commandList->EndRenderPass();
    }
   */
}