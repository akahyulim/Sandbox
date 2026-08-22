#include "pch.h"
#include "Renderer.h"
#include "TextureManager.h"
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

namespace Dive
{
	Renderer::Renderer(Graphics* graphics, uint32_t width, uint32_t height)
		: m_graphics(graphics),
		m_width(width),
		m_height(height)
	{
		createDepthStencilStates();
        createRasterizerStates();
        createBlendStates();
		createSamplers();
        createBuffers();
        loadTextures();

		// create other resources
        createResolutionDependantResources(width, height);
	}

	Renderer::~Renderer() = default;
	
	void Renderer::Update(Scene* scene)
	{
        if (!scene)
            return;

        bindGlobals();

        auto camera = scene->GetCamera()->GetComponent<Camera>();
        m_frameData.backgroundColor = DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);
        m_frameData.position = DirectX::XMFLOAT4(
            camera->GetTransform()->GetPosition().x,
            camera->GetTransform()->GetPosition().y,
            camera->GetTransform()->GetPosition().z,
            1.0f);
        m_frameData.viewMatrix = DirectX::XMMatrixTranspose(camera->GetViewMatrix());
        m_frameData.projMatrix = DirectX::XMMatrixTranspose(camera->GetProjectionMatrix());
        m_frameData.viewProjMatrix = DirectX::XMMatrixTranspose(camera->GetViewProjMatrix());
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

        // pass들 내부에는 Graphics::BeginRenderPass()에 각각의 RenderPassDesc를 사용
        // Render Target, Depth와 clear값이 들어있다.
        passTest(scene);
        passGBuffer();
        passPicking();
        passAmbient();
        passDeferredLighting();
        passForward();
        passSkybox(scene);
	}

    void Renderer::ResolveToOffScreenTexture()
    {
        // 이 부분은 매개변수화해서 BeginRenderPass로 넘겨야 한다.
        {
            auto rtv = m_offScreenRenderTarget->GetRenderTargetView();
            float clearColor[] = { 0.0f ,0.0f, 0.0f, 0.0f };
            m_graphics->GetDeviceContext()->ClearRenderTargetView(rtv, clearColor);
            m_graphics->GetDeviceContext()->OMSetRenderTargets(1, &rtv, nullptr);
        }

        m_graphics->BeginRenderPass();

        auto srv = m_ldrRenderTarget->GetShaderResourceView();
        m_graphics->SetShaderResourceView(eShaderStage::PS, 30, &srv);  // 원래 srv slot enum class가 없었나...
        ShaderManager::GetInst().GetShaderProgram(eShaderPrograms::Resolve)->Bind(m_graphics);
        m_graphics->SetTopology(ePrimitiveTopology::TriangleStrip);
        m_graphics->SetVertexBuffer(nullptr);
        m_graphics->Draw(4);

        m_graphics->EndRenderPass();
    }

    void Renderer::ResolveToBackbuffer()
    {
        m_graphics->SetBackbuffer();

        auto srv = m_ldrRenderTarget->GetShaderResourceView();
        m_graphics->SetShaderResourceView(eShaderStage::PS, 30, &srv);  // 원래 srv slot enum class가 없었나...
        ShaderManager::GetInst().GetShaderProgram(eShaderPrograms::Resolve)->Bind(m_graphics);
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
            desc.MaxAnisotropy = 1;
            desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
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

        const SimpleVertex vertices[] = 
		{
			DirectX::XMFLOAT3{ -1.5f, -1.5f,  1.5f },
			DirectX::XMFLOAT3{  1.5f, -1.5f,  1.5f },
			DirectX::XMFLOAT3{  1.5f,  1.5f,  1.5f },
			DirectX::XMFLOAT3{ -1.5f,  1.5f,  1.5f },
			DirectX::XMFLOAT3{ -1.5f, -1.5f, -1.5f },
			DirectX::XMFLOAT3{  1.5f, -1.5f, -1.5f },
			DirectX::XMFLOAT3{  1.5f,  1.5f, -1.5f },
			DirectX::XMFLOAT3{ -1.5f,  1.5f, -1.5f }
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

        m_cubeVB = std::make_unique<VertexBuffer>(m_graphics, sizeof(SimpleVertex), 8, vertices);
        m_cubeIB = std::make_unique<IndexBuffer>(m_graphics, eFormat::R16_UINT, 36, indices);
    }

    void Renderer::createResolutionDependantResources(uint32_t width, uint32_t height)
    {
        createRenderTargets(width, height);
        createGBuffer(width, height);
        createRenderPasses(width, height);
    }

    // 일단 LDR로 통일
    void Renderer::createRenderTargets(uint32_t width, uint32_t height)
    {
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = width;
        desc.Height = height;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET; 
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.MiscFlags = 0;

        m_ldrRenderTarget = std::make_unique<RenderTexture>(m_graphics, desc);
        m_offScreenRenderTarget = std::make_unique<RenderTexture>(m_graphics, desc);

        desc.Format = DXGI_FORMAT_R16_TYPELESS;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;

        m_depthTarget = std::make_unique<RenderTexture>(m_graphics, desc);
    }

    void Renderer::createGBuffer(uint32_t width, uint32_t height)
    {
    }

    void Renderer::createRenderPasses(uint32_t width, uint32_t height)
    {
        // createRenderTargets에서 생성된 rtv들과 dsv들
        // 그리고 clearValue 등을 묶어 dsec를 구성한다.
        // width, height는 desc에서 vieewport용으로 저장한다.
        // 그리고 이 desc는 pass에 매칭되는 멤버변수로 관리한다.
    }

    void Renderer::loadTextures()
    {
    }

    // 다시 adria를 살표본 결과 이 부분은 문제가 없다.
    // 그리고 재미나이도 이 방법을 추천하고 있다.
    // 즉, 상수버퍼의 바인딩은 한 번으로 족하다.
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
            //m_cbMaterial->Bind(m_graphics, eShaderStage::PS, static_cast<uint32_t>(eConstantBuffer::Material));
            //m_cbLight->Bind(m_graphics, eShaderStage::PS, static_cast<uint32_t>(eConstantBuffer::Light));

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

    void Renderer::passTest(Scene* scene)
    {
        // 원래는 RenderPassDesc에 rtv, dsv, clearValue를 구성하고
        // 아래의 BeginRenderPass에 전달하면
        // 내부에서 rtv, dsv를 설정 및 클리어한다.
        {
            auto rtv = m_ldrRenderTarget->GetRenderTargetView();
            auto dsv = m_depthTarget->GetDetphStencilView();
            float clearColor[] = { 0.4f, 0.4f, 1.0f, 1.0f };
            m_graphics->GetDeviceContext()->ClearRenderTargetView(rtv, clearColor);
            m_graphics->GetDeviceContext()->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
            m_graphics->GetDeviceContext()->OMSetRenderTargets(1, &rtv, dsv);

            m_graphics->SetViewport(scene->GetCamera()->GetComponent<Camera>()->GetViewport());
            //m_graphics->BeginRenderPass();

            m_objectData.model = DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity());
            m_cbObject->Update(m_graphics, m_objectData);

            m_graphics->SetRasterizerState(m_rasterizerStates[(size_t)eRasterizerState::FillSolid_CullBack].Get());
            m_graphics->SetDepthStencilState(m_depthStencilStates[(size_t)eDepthStencilState::Default].Get(), 0);

            m_graphics->SetTopology(ePrimitiveTopology::TriangleList);

            ShaderManager::GetInst().GetShaderProgram(eShaderPrograms::Test)->Bind(m_graphics);

            //m_graphics->SetVertexBuffer(m_cubeVB.get());
            //m_graphics->SetIndexBuffer(m_cubeIB.get());
            //m_graphics->DrawIndexed(m_cubeIB->GetCount());
            m_graphics->SetVertexBuffer(nullptr);
            m_graphics->Draw(3);

            //m_graphics->EndRenderPass();
        }
    }

    void Renderer::passGBuffer()
    {
    }
    
    void Renderer::passPicking()
    {
    }
    
    void Renderer::passAmbient()
    {
    }

    void Renderer::passDeferredLighting()
    {
    }
    
    void Renderer::passForward()
    {
    }

    // RenderPassDesc가 없다. 간혹 이런것들이 존재한다.
    void Renderer::passSkybox(Scene* scene)
    {
        // 임시: SetViewport용
        //m_graphics->SetViewport(scene->GetCamera()->GetComponent<Camera>()->GetViewport());
        //m_graphics->BeginRenderPass();

        m_graphics->SetRasterizerState(m_rasterizerStates[(size_t)eRasterizerState::FillSolid_CullNone].Get());
        m_graphics->SetDepthStencilState(m_depthStencilStates[(size_t)eDepthStencilState::Skybox].Get(), 0);

        ShaderManager::GetInst().GetShaderProgram(eShaderPrograms::Skybox)->Bind(m_graphics);
        
        auto& envData = scene->GetEnviroment();
        auto srv = TextureManager::GetInst().GetTextureView(envData.skyboxCubemap);
        m_graphics->SetShaderResourceView(eShaderStage::PS, 21, &srv);

        m_graphics->SetTopology(ePrimitiveTopology::TriangleList);
        m_graphics->SetVertexBuffer(nullptr);
        m_graphics->Draw(36);
        
        m_graphics->SetRasterizerState(nullptr);
        m_graphics->SetDepthStencilState(nullptr, 0);

        //m_graphics->EndRenderPass();
    }
}