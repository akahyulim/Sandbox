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
#include "Scene/Components/Light.h"

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
        auto transform = camera->GetTransform();
        camera->SetAspectRatio(static_cast<float>(m_width), static_cast<float>(m_height));

        {    
            FrameData frameData = {};

            frameData.view = DirectX::XMMatrixTranspose(camera->GetViewMatrix());
            frameData.projection = DirectX::XMMatrixTranspose(camera->GetProjectionMatrix());
            frameData.viewProjection = DirectX::XMMatrixTranspose(camera->GetViewProjMatrix());
            frameData.inverseViewProjection = DirectX::XMMatrixTranspose(camera->GetInverseViewProjMatrix());
            frameData.cameraPosition = camera->GetPosition();
            frameData.cameraForward = camera->GetForward();

            frameData.screenResolution.x = static_cast<float>(m_width);
            frameData.screenResolution.y = static_cast<float>(m_height);
            frameData.mousePosition = m_mousePosition;

            m_cbFrame->Update(m_graphics, frameData);
        }

        {
            const auto& lights = scene->GetLightQueue();

            static LightConstants lightConstants = {};

            bool anyLightChanged = false;

            size_t currentCount = std::min(lights.size(), size_t(32));
            if (lightConstants.lightCount != static_cast<uint32_t>(currentCount))
            {
                lightConstants.lightCount = static_cast<uint32_t>(currentCount);
                anyLightChanged = true;
            }

            for (size_t i = 0; i < currentCount; ++i)
            {
                auto lightCom = lights[i]->GetComponent<Light>();
                if (lightCom->IsDirty())
                {
                    lightConstants.lights[i] = lightCom->GetLightData();
                    lightCom->ClearDirty();
                    anyLightChanged = true;
                }
            }

            if (anyLightChanged)
            {
                m_cbLight->Update(m_graphics, lightConstants);
            }
        }

        updateWeather();

        // 추후 분리
        {
            m_opaques.clear();
            m_transparents.clear();
            m_gizmos.clear();

            //auto cameraFrustum = camera->GetFrustum();
            auto cameraPos = camera->GetTransform()->GetPosition();

            // 1. 일반 오브젝트 순회
            for (auto& renderable : scene->GetRenderables())
            {
                auto renderer = renderable->GetComponent<MeshRenderer>();
                if (!renderer) continue;

                // 💡 [1단계] 프러스텀 컬링: 화면에 안 보이면 스킵!
                //if (!cameraFrustum.Contains(renderer->GetBoundingBox()))
                //    continue; 

                // 💡 [2단계] 분류
                auto material = renderer->GetMaterial();
                if (material->IsTransparent())
                    m_transparents.push_back(renderable);
                else
                    m_opaques.push_back(renderable);
            }

            // 라이트 기즈모도 동일하게 컬링 후 수집...

            // 💡 [3단계] 보이는 것들만 대상으로 반투명 정렬 수행
            auto sortByDistanceDescending = [cameraPos](auto* a, auto* b) {
                // 오브젝트의 월드 포지션 가져오기 (프로젝트에 맞는 메서드로 수정)
                auto posA = a->GetTransform()->GetPosition(); // 혹은 a->GetPosition()
                auto posB = b->GetTransform()->GetPosition();

                // 직접 거리의 제곱(Distance Squared) 계산
                float dxA = posA.x - cameraPos.x;
                float dyA = posA.y - cameraPos.y;
                float dzA = posA.z - cameraPos.z;
                float distA = (dxA * dxA) + (dyA * dyA) + (dzA * dzA);

                float dxB = posB.x - cameraPos.x;
                float dyB = posB.y - cameraPos.y;
                float dzB = posB.z - cameraPos.z;
                float distB = (dxB * dxB) + (dyB * dyB) + (dzB * dzB);

                // 먼 것부터 앞으로 오도록 정렬 (Back-to-Front)
                return distA > distB;
                };

            std::sort(m_transparents.begin(), m_transparents.end(), sortByDistanceDescending);
            //std::sort(m_gizmos.begin(), m_gizmos.end(), sortByDistanceDescending);
        }
	}
	
    // Render에는 RenderSettings가 전달
    // Editor의 멤버 변수이며 Editor::Run -> Engine::Run -> Render -> Renderer::Render 순으로 전달
    // 각종 렌더링 옵션들로 구성되어 있으며 Editor에서 취사 선택이 가능한 형태
    // 게임에서 그래픽스 옵션이라고 볼 수 있다.
	void Renderer::Render(Scene* scene)
    {	
        if (!scene)
            return;

        passGBuffer();
        passDeferred();
        passSky(scene);
        passForward();
        //passPostProcessing();
	}

    void Renderer::ResolveToOffScreenTexture()
    {
        m_graphics->BeginRenderPass(m_offScreenResolvePass);

        m_graphics->SetViewport(m_offScreenRenderTarget->GetWidth(), m_offScreenRenderTarget->GetHeight());
        auto srv = m_ldrRenderTarget->GetShaderResourceView();
        m_graphics->SetShaderResourceView(eShaderStage::PS, 13, &srv);  // 원래 srv slot enum class가 없었나...
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
        m_graphics->SetShaderResourceView(eShaderStage::PS, 13, &srv);  // 원래 srv slot enum class가 없었나...
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

    void Renderer::SetSelectedObjectID(uint32_t objectID)
    {
        if (m_lastSelectedObjectID != objectID)
        {
            m_lastSelectedObjectID = objectID;

            SelectedObjectData data = { objectID, 0, 0, 0 };
            m_cbSelectedObject->Update(m_graphics, data);
        }
    }

    // id만 활용할 거라면 normal, depth 다 필요없다.
    void Renderer::ProcessPicking()
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
        // Forward
        {
            D3D11_BLEND_DESC desc{};
            desc.AlphaToCoverageEnable = false;
            desc.IndependentBlendEnable = true;

            D3D11_RENDER_TARGET_BLEND_DESC colorBlendDesc = {};
            colorBlendDesc.BlendEnable = true;
            colorBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
            colorBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            colorBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
            colorBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
            colorBlendDesc.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
            colorBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
            colorBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            desc.RenderTarget[0] = colorBlendDesc;

            D3D11_RENDER_TARGET_BLEND_DESC idBlendDesc = {};
            idBlendDesc.BlendEnable = false; // 정수형 ID 타겟은 블렌딩 끄기
            idBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            desc.RenderTarget[1] = idBlendDesc;

            m_blendStates[static_cast<size_t>(eBlendState::Forward)] =
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

        // Skysphere (Equirectangular 2D 맵 전용)
        {
            ZeroMemory(&desc, sizeof(desc));
            desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // 부드러운 선형 필터링
            desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;  // 좌우 끝은 자연스럽게 이어지도록 Wrap
            desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP; // 상하 극점은 찢어지지 않게 Clamp
            desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
            desc.MaxAnisotropy = 1;
            desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
            desc.MinLOD = 0;
            desc.MaxLOD = D3D11_FLOAT32_MAX;

            m_samplerStates[static_cast<size_t>(eSamplerState::SkySphere)] =
                m_graphics->CreateSamplerState(desc);
        }
    }

    void Renderer::createBuffers()
    {
        m_cbFrame = std::make_unique<ConstantBuffer<FrameData>>(m_graphics); 
        m_cbObject = std::make_unique<ConstantBuffer<ObjectData>>(m_graphics);
        m_cbWeather = std::make_unique<ConstantBuffer<WeatherData>>(m_graphics);
        m_cbLight = std::make_unique<ConstantBuffer<LightConstants>>(m_graphics);

        m_cbSelectedObject = std::make_unique<ConstantBuffer<SelectedObjectData>>(m_graphics);

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
            RenderTargetDesc albedoDesc{};
            albedoDesc.RenderTargetView = m_gbuffer[static_cast<size_t>(eGBufferType::AlbedoRoughness)]->GetRenderTargetView();
            albedoDesc.ClearColor[0] = 0.0f;
            albedoDesc.ClearColor[1] = 0.0f;
            albedoDesc.ClearColor[2] = 0.0f;
            albedoDesc.ClearColor[3] = 0.0f;
            albedoDesc.AccessType = eLoadAccessOp::Clear;

            RenderTargetDesc normalDesc{};
            normalDesc.RenderTargetView = m_gbuffer[static_cast<size_t>(eGBufferType::NormalMetallic)]->GetRenderTargetView();
            normalDesc.ClearColor[0] = 0.0f;
            normalDesc.ClearColor[1] = 0.0f;
            normalDesc.ClearColor[2] = 0.0f;
            normalDesc.ClearColor[3] = 0.0f;
            normalDesc.AccessType = eLoadAccessOp::Clear;

            RenderTargetDesc emissiveDesc{};
            emissiveDesc.RenderTargetView = m_gbuffer[static_cast<size_t>(eGBufferType::Emissive)]->GetRenderTargetView();
            emissiveDesc.ClearColor[0] = 0.0f;
            emissiveDesc.ClearColor[1] = 0.0f;
            emissiveDesc.ClearColor[2] = 0.0f;
            emissiveDesc.ClearColor[3] = 0.0f;
            emissiveDesc.AccessType = eLoadAccessOp::Clear;

            RenderTargetDesc objectIDDesc{};
            objectIDDesc.RenderTargetView = m_gbuffer[static_cast<size_t>(eGBufferType::ObjectID)]->GetRenderTargetView();
            objectIDDesc.ClearColor[0] = 0.0f;
            objectIDDesc.ClearColor[1] = 0.0f;
            objectIDDesc.ClearColor[2] = 0.0f;
            objectIDDesc.ClearColor[3] = 0.0f;
            objectIDDesc.AccessType = eLoadAccessOp::Clear;

            DepthStencilDesc dsDesc{};
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
            RenderTargetDesc rtDesc{};
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

        // Forward
        {
            RenderTargetDesc rtDesc{};
            rtDesc.RenderTargetView = m_ldrRenderTarget->GetRenderTargetView();
            rtDesc.AccessType = eLoadAccessOp::Load;

            RenderTargetDesc objectIDDesc{};
            objectIDDesc.RenderTargetView = m_gbuffer[static_cast<size_t>(eGBufferType::ObjectID)]->GetRenderTargetView();
            objectIDDesc.AccessType = eLoadAccessOp::Load;

            DepthStencilDesc dsDesc{};
            dsDesc.DepthStencilView = m_gbuffer[static_cast<size_t>(eGBufferType::Depth)]->GetDetphStencilView();
            dsDesc.AccessType = eLoadAccessOp::Load;

            RenderPassDesc desc{};
            desc.renderTargetDescs.push_back(rtDesc);
            desc.renderTargetDescs.push_back(objectIDDesc);
            desc.depthStencilDesc = dsDesc;
            m_forwardPass = desc;
        }

        // Skybox
        {
            RenderTargetDesc rtDesc{};
            rtDesc.RenderTargetView = m_ldrRenderTarget->GetRenderTargetView();
            rtDesc.AccessType = eLoadAccessOp::Load;

            DepthStencilDesc dsDesc{};
            dsDesc.DepthStencilView = m_gbuffer[static_cast<size_t>(eGBufferType::Depth)]->GetDetphStencilView();
            dsDesc.AccessType = eLoadAccessOp::Load;

            RenderPassDesc desc{};
            desc.renderTargetDescs.push_back(rtDesc);
            desc.depthStencilDesc = dsDesc;
            m_skyboxPass = desc;
        }

        // offScreen Resolve
        {
            RenderTargetDesc rtDesc{};
            rtDesc.RenderTargetView = m_offScreenRenderTarget->GetRenderTargetView();
            rtDesc.ClearColor[0] = 0.0f;
            rtDesc.ClearColor[1] = 0.0f;
            rtDesc.ClearColor[2] = 0.0f;
            rtDesc.ClearColor[3] = 0.0f;
            rtDesc.AccessType = eLoadAccessOp::Clear;

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
            m_cbWeather->Bind(m_graphics, eShaderStage::VS, static_cast<uint32_t>(eConstantBuffer::Weather));

            // hs
            m_cbFrame->Bind(m_graphics, eShaderStage::HS, static_cast<uint32_t>(eConstantBuffer::Frame));

            // ds
            m_cbFrame->Bind(m_graphics, eShaderStage::DS, static_cast<uint32_t>(eConstantBuffer::Frame));

            // ps
            m_cbFrame->Bind(m_graphics, eShaderStage::PS, static_cast<uint32_t>(eConstantBuffer::Frame));
            m_cbObject->Bind(m_graphics, eShaderStage::PS, static_cast<uint32_t>(eConstantBuffer::Object));
            m_cbWeather->Bind(m_graphics, eShaderStage::PS, static_cast<uint32_t>(eConstantBuffer::Weather));
            m_cbLight->Bind(m_graphics, eShaderStage::PS, static_cast<uint32_t>(eConstantBuffer::Light));
            m_cbSelectedObject->Bind(m_graphics, eShaderStage::PS, static_cast<uint32_t>(eConstantBuffer::SelectedObject));

            // cs
            m_cbFrame->Bind(m_graphics, eShaderStage::CS, static_cast<uint32_t>(eConstantBuffer::Frame));

            ID3D11SamplerState* samplers[static_cast<size_t>(eSamplerState::Count)] =
            {
                m_samplerStates[static_cast<size_t>(eSamplerState::WrapLinear)].Get(),
                m_samplerStates[static_cast<size_t>(eSamplerState::ClampPoint)].Get(),
                m_samplerStates[static_cast<size_t>(eSamplerState::ClampLinear)].Get(),
                m_samplerStates[static_cast<size_t>(eSamplerState::Skybox)].Get(),
                m_samplerStates[static_cast<size_t>(eSamplerState::SkySphere)].Get(),
                m_samplerStates[static_cast<size_t>(eSamplerState::ShadowCompare)].Get()
            };

            deviceContext->DSSetSamplers(0, static_cast<UINT>(eSamplerState::Count), samplers);
            deviceContext->PSSetSamplers(0, static_cast<UINT>(eSamplerState::Count), samplers);

            called = true;
        }
    }

    // 역시 Enviroment가 어울린다.
    void Renderer::updateWeather()
    {
        // 이건 멤버 변수가 더 어울릴 수 있다.
        // 현재는 데이터들을 개별 메서드로 전달, 입력받고 있기 때문이다.
        WeatherData data;

        DirectX::XMVECTOR dir = DirectX::XMLoadFloat4(&m_lightDir);
        DirectX::XMVector4Normalize(dir);
        DirectX::XMStoreFloat4(&data.lightDir, dir);
  
        data.lightColor = m_lightColor;
        data.ambientColor = m_ambientColor;
        data.skyColor = m_skyColor;
        m_cbWeather->Update(m_graphics, data);
    }
    
    void Renderer::passGBuffer()
    {
        m_graphics->BeginRenderPass(m_gbufferPass);

        m_graphics->SetViewport(m_ldrRenderTarget->GetWidth(), m_ldrRenderTarget->GetHeight());

        m_graphics->SetRasterizerState(m_rasterizerStates[(size_t)eRasterizerState::FillSolid_CullBack].Get());
        m_graphics->SetDepthStencilState(m_depthStencilStates[(size_t)eDepthStencilState::Default].Get(), 0);

        ObjectData data{};

        for (auto opaque : m_opaques)
        {
            if (!opaque->IsActive())
                continue;

            auto transform = opaque->GetTransform();
            auto staticMesh = opaque->GetComponent<MeshRenderer>();
            auto material = staticMesh->GetMaterial();
            
            bool hasTessellation = material->HasMap(eMapType::Displacement);

            if (hasTessellation)
            {
                ShaderManager::Get().GetShaderProgram(eShaderPrograms::GBufferTessellation)->Bind(m_graphics);
                m_graphics->SetTopology(ePrimitiveTopology::PatchList_3_ControlPoints);
            }
            else
            {
                ShaderManager::Get().GetShaderProgram(eShaderPrograms::GBuffer)->Bind(m_graphics);
                m_graphics->SetTopology(ePrimitiveTopology::TriangleList);
            }

            data.model = DirectX::XMMatrixTranspose(transform->GetWorldMatrix());
            data.id = staticMesh->GetObjectID();
            m_cbObject->Update(m_graphics, data);

            staticMesh->Draw(m_graphics);
        }

        m_graphics->EndRenderPass();
    }
    
    void Renderer::passDeferred()
    {
        m_graphics->BeginRenderPass(m_deferredLightingPass);

        m_graphics->SetViewport(m_ldrRenderTarget->GetWidth(), m_ldrRenderTarget->GetHeight());

        m_graphics->SetRasterizerState(m_rasterizerStates[(size_t)eRasterizerState::FillSolid_CullNone].Get());
        m_graphics->SetDepthStencilState(nullptr, 0);
        m_graphics->SetTopology(ePrimitiveTopology::TriangleStrip);

        ShaderManager::Get().GetShaderProgram(eShaderPrograms::Deferred)->Bind(m_graphics);

        // SetGBufferSRV 같은 걸 만드는 게 나을 듯하다.
        auto albedoSrv = m_gbuffer[static_cast<size_t>(eGBufferType::AlbedoRoughness)]->GetShaderResourceView();
        auto normalSrv = m_gbuffer[static_cast<size_t>(eGBufferType::NormalMetallic)]->GetShaderResourceView();
        auto idSrv = m_gbuffer[static_cast<size_t>(eGBufferType::ObjectID)]->GetShaderResourceView();
        auto depthSrv = m_gbuffer[static_cast<size_t>(eGBufferType::Depth)]->GetShaderResourceView();
        m_graphics->SetShaderResourceView(eShaderStage::PS, 6, &albedoSrv);
        m_graphics->SetShaderResourceView(eShaderStage::PS, 7, &normalSrv);
        m_graphics->SetShaderResourceView(eShaderStage::PS, 9, &idSrv);
        m_graphics->SetShaderResourceView(eShaderStage::PS, 10, &depthSrv);

        m_graphics->SetVertexBuffer(nullptr);
        m_graphics->Draw(4);

        m_graphics->EndRenderPass();
    }
    
    void Renderer::passSky(Scene* scene)
    {
        m_graphics->BeginRenderPass(m_skyboxPass);

        m_graphics->SetViewport(m_ldrRenderTarget->GetWidth(), m_ldrRenderTarget->GetHeight());

        m_graphics->SetRasterizerState(m_rasterizerStates[(size_t)eRasterizerState::FillSolid_CullNone].Get());
        m_graphics->SetDepthStencilState(m_depthStencilStates[(size_t)eDepthStencilState::Skybox].Get(), 0);

        if (m_skyMode == eSkyMode::Skybox)
        {
            ShaderManager::Get().GetShaderProgram(eShaderPrograms::Skybox)->Bind(m_graphics);

            auto& envData = scene->GetEnviroment();
            auto srv = TextureManager::Get().GetTextureView(envData.skyboxCubemap);
            m_graphics->SetShaderResourceView(eShaderStage::PS, 11, &srv);
        }
        else if (m_skyMode == eSkyMode::SkySphere)
        {
            ShaderManager::Get().GetShaderProgram(eShaderPrograms::SkySphere)->Bind(m_graphics);

            auto& envData = scene->GetEnviroment();
            auto srv = TextureManager::Get().GetTextureView(envData.skyboxCubemap);
            m_graphics->SetShaderResourceView(eShaderStage::PS, 12, &srv);
        }
        else
        {
            ShaderManager::Get().GetShaderProgram(eShaderPrograms::UniformSky)->Bind(m_graphics);
        }

        ObjectData data{};
        auto camera = scene->GetCamera();
        data.model = DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslationFromVector(camera->GetTransform()->GetPositionVector()));
        m_cbObject->Update(m_graphics, data);

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
        m_graphics->BeginRenderPass(m_forwardPass);

        m_graphics->SetViewport(m_ldrRenderTarget->GetWidth(), m_ldrRenderTarget->GetHeight());

        m_graphics->SetRasterizerState(m_rasterizerStates[(size_t)eRasterizerState::FillSolid_CullNone].Get());
        m_graphics->SetDepthStencilState(m_depthStencilStates[(size_t)eDepthStencilState::Transparent].Get(), 0);
        m_graphics->SetBlendState(m_blendStates[(size_t)eBlendState::Forward ].Get());

        {
            // gizmo

            // tnrasparent
            ObjectData data{};
            for (auto transparent : m_transparents)
            {
                if (!transparent->IsActive())
                    continue;

                auto transform = transparent->GetTransform();
                auto staticMesh = transparent->GetComponent<MeshRenderer>();
                auto material = staticMesh->GetMaterial();
                /*
                bool hasTessellation = material->HasMap(eMapType::Displacement);

                if (hasTessellation)
                {
                    //ShaderManager::Get().GetShaderProgram(eShaderPrograms::ForwardTessellation)->Bind(m_graphics);
                    //m_graphics->SetTopology(ePrimitiveTopology::PatchList_3_ControlPoints);
                }
                else*/
                {
                    ShaderManager::Get().GetShaderProgram(eShaderPrograms::Forward)->Bind(m_graphics);
                    m_graphics->SetTopology(ePrimitiveTopology::TriangleList);
                }

                data.model = DirectX::XMMatrixTranspose(transform->GetWorldMatrix());
                data.id = staticMesh->GetObjectID();
                m_cbObject->Update(m_graphics, data);

                staticMesh->Draw(m_graphics);
            }
        }

        m_graphics->SetRasterizerState(nullptr);
        m_graphics->SetDepthStencilState(nullptr, 0);
        m_graphics->SetBlendState(nullptr);

        m_graphics->EndRenderPass();
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