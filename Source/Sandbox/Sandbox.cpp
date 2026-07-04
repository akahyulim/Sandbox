#include "pch.h"
#include "Sandbox.h"
#include "Core/Window.h"
#include "Core/EventDispatcher.h"
#include "Core/Timer.h"
#include "Graphics/Graphics.h"
#include "Renderer/Renderer.h"
#include "Input/Input.h"
#include "Scene/Scene.h"
#include "Scene/Components/Camera.h"
#include "Scene/Components/Transform.h"
#include "Scene/Components/MeshRenderer.h"
#include "Scene/Components/Light.h"
#include "Resource/ShaderManager.h"
#include "Resource/ResourceManager.h"
#include "Resource/Texture2D.h"
#include "Resource/RenderTexture.h"
#include "Resource/Material.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT32 msg, WPARAM wParam, LPARAM lParam);

namespace Dive
{
    namespace
    {
        constexpr float BOOST_SPEED = 10.0f;
        constexpr float MIN_SPEED = 0.5f;
        constexpr float MAX_SPEED = 99.0f;

        Graphics* s_graphics = nullptr;

        static LRESULT CALLBACK SandboxMessageHandler(HWND hWnd, UINT32 msg, WPARAM wParam, LPARAM lParam)
        {
            ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);

            switch (msg)
            {
            case WM_SIZE:
            {
                DV_FIRE_EVENT(Dive::eEventType::WindowResized);
                if (s_graphics)
                    s_graphics->OnResizeViews();
                return 0;
            }
            case WM_CLOSE:
            {
                Dive::Window::GetInst().Close();
                return 0;
            }
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
            }

            return ::DefWindowProc(hWnd, msg, wParam, lParam);
        }
    }

    Sandbox::Sandbox()
    {
        m_timer = std::make_unique<Timer>();
        m_graphics = std::make_unique<Graphics>();
        m_renderer = std::make_unique<Renderer>();

        m_scene = std::make_unique<Scene>();
    }

    Sandbox::~Sandbox()
    {
    }

    bool Sandbox::Initialize()
    {
        if (!Window::GetInst().Initialize())
            return false;
        Window::GetInst().SetMessageCallback((LONG_PTR)SandboxMessageHandler);

        if (!Input::GetInst().Initialize(Window::GetInst().GetWindowHandle()))
            return false;

        if (!m_graphics->Initialize(
            Window::GetInst().GetWindowHandle(),
            Window::GetInst().GetWidth(),
            Window::GetInst().GetHeight(),
            Window::GetInst().IsWindowed()
        ))
            return false;
        s_graphics = m_graphics.get();

        if (!m_renderer->Initialize(m_graphics.get()))
            return false;

        // ImGui 초기화
        {
            // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

            // Setup Dear ImGui style
            ImGui::StyleColorsDark();
            //ImGui::StyleColorsLight();
            //ImGui::StyleColorsClassic();

            // Setup Platform/Renderer backends
            ImGui_ImplWin32_Init(Window::GetInst().GetWindowHandle());
            ImGui_ImplDX11_Init(m_graphics->GetDevice(), m_graphics->GetDeviceContext());
        }

        {
            if (!ShaderManager::GetInst().Initialize(m_graphics.get()))
                return false;
        }

        {
            if (!ResourceManager::GetInst().Initialize(m_graphics.get()))
                return false;

            //ResourceManager::GetInst().Load<Texture2D>("Assets/Textures/DokeV.jpeg");
            ResourceManager::GetInst().Load<Texture2D>("Assets/Textures/stone01.tga");
            ResourceManager::GetInst().Load<Texture2D>("Assets/Textures/normal01.tga");
        }

        // scene 초기화
        // => New Scene으로 메서드화?
        {
            auto plane = m_scene->AddPresetObject(ePresetType::Plane);
            auto mat = plane->GetComponent<MeshRenderer>()->GetMaterial();
            mat->SetTexture(eTextureMapType::Diffuse, "Assets/Textures/stone01.tga");
            mat->SetTexture(eTextureMapType::Normal, "Assets/Textures/normal01.tga");
            mat->SetTiling(5.0f, 5.0f);

            auto* mainCamera = m_scene->GetMainCamera();
            auto* transform = mainCamera->GetTransform();
            transform->SetPosition(0.0f, 3.0f, -5.0f);
            transform->LookAt(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));

            auto* cameraCom = mainCamera->GetComponent<Camera>();
            cameraCom->SetViewport(0.0f, 0.0f, (float)m_graphics->GetWidth(), (float)m_graphics->GetHeight());
            auto targetTexture = m_graphics->CreateRenderTexture(m_graphics->GetWidth(), m_graphics->GetHeight());
            cameraCom->SetTargetTexture(targetTexture);
        }

        m_timer->Start();

        return true;
    }

    void Sandbox::Run()
    {
        while (Window::GetInst().Run())
        {
            m_timer->Tick();
            float dt = m_timer->GetDeltaTimeMS();

            Input::GetInst().Update();

            this->cameraControll(dt);

            m_scene->Update(dt);
            m_scene->PrepareRenderChannels();

            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            // 단축키 가동 (방어 코드)
            if (!ImGui::IsAnyItemActive())
            {
                if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Q))
                {
                    Window::GetInst().Close();
                }
            }

            // 현재 메인 윈도우 창의 위치와 크기 확보
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);

            ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoBringToFrontOnFocus;

            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

            if (ImGui::Begin("MainEditorCanvas", nullptr, windowFlags))
            {
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();

                if (ImGui::BeginPopupContextWindow("CanvasContextMenu", ImGuiPopupFlags_MouseButtonRight))
                {
                    if (ImGui::MenuItem("New"))
                    {
                        m_scene->ClearAll();
                    }

                    if (ImGui::MenuItem("Open"))
                    {
                        //m_scene->LoadFromFile();
                    }

                    ImGui::Separator();

                    ImGui::MenuItem("Environment", nullptr, &m_showEnvDiralog, m_scene != nullptr);
                     

                    ImGui::Separator();

                    if (ImGui::BeginMenu("3D Object"))
                    {
                        if (ImGui::MenuItem("Triangle", nullptr, nullptr, m_scene != nullptr))
                        {
                            auto triangle = m_scene->AddPresetObject(ePresetType::Triangle);
                            triangle->GetTransform()->SetPosition(0.0f, 0.5f, 0.0f);
                        }
                        if (ImGui::MenuItem("Quad", nullptr, nullptr, m_scene != nullptr))
                        {
                            auto quad = m_scene->AddPresetObject(ePresetType::Quad);
                            quad->GetTransform()->SetPosition(0.0f, 0.5f, 0.0f);
                        }
                        if (ImGui::MenuItem("Cube", nullptr, nullptr, m_scene != nullptr))
                        {
                            auto cube = m_scene->AddPresetObject(ePresetType::Cube);
                            cube->GetTransform()->SetPosition(0.0f, 0.5f, 0.0f);
                        }
                        if (ImGui::MenuItem("Sphere", nullptr, nullptr, m_scene != nullptr))
                        {
                            auto sphere = m_scene->AddPresetObject(ePresetType::Sphere);
                            sphere->GetTransform()->SetPosition(0.0f, 0.5f, 0.0f);
                        }
                        if (ImGui::MenuItem("Capsule", nullptr, nullptr, m_scene != nullptr))
                        {
                            auto capsule = m_scene->AddPresetObject(ePresetType::Capsule);
                            capsule->GetTransform()->SetPosition(0.0f, 1.0f, 0.0f);
                        }
                        if (ImGui::MenuItem("Model", nullptr, nullptr, m_scene != nullptr))
                        {
                        }
                        ImGui::EndMenu();
                    }

                    ImGui::Separator();

                    if (ImGui::MenuItem("Copy", nullptr, nullptr, m_scene->GetSelectedObject() != nullptr))
                    {
                    }
                    if (ImGui::MenuItem("Paste", nullptr, nullptr, m_scene->GetSelectedObject() != nullptr))
                    {
                    }
                    if (ImGui::MenuItem("Delete", nullptr, nullptr, m_scene->GetSelectedObject() != nullptr))
                    {
                    }

                    ImGui::Separator();

                    if (ImGui::MenuItem("Save"))
                    {
                    }
                    if (ImGui::MenuItem("Save As..."))
                    {
                    }

                    ImGui::Separator();

                    if (ImGui::MenuItem("Exit", "Ctrl+Q"))
                    {
                        Window::GetInst().Close();
                    }

                    ImGui::EndPopup();
                }
            }
            ImGui::End();

            if (m_showEnvDiralog)
            {
                const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
                ImVec2 windowPos = ImVec2(mainViewport->WorkPos.x + mainViewport->WorkSize.x - 320.0f, mainViewport->WorkPos.y + 20.0f);
                ImVec2 windowSize = ImVec2(300.0f, 200.0f);

                ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);

                if (ImGui::Begin("Environment", &m_showEnvDiralog, ImGuiWindowFlags_NoSavedSettings))
                {
                    // 🌟 변경 감지를 위해 하나로 묶기
                    bool isChanged = false;

                    ImGui::Text("Sky");
                    ImGui::Separator();
                    m_enviromentData.skyColor = m_scene->GetMainCamera()->GetComponent<Camera>()->GetClearColor();
                    if (ImGui::ColorEdit3("Sky Color", &m_enviromentData.skyColor.r)) isChanged = true;

                    ImGui::Text("Directional Light");
                    ImGui::Separator();

                    auto dirLight = m_scene->GetDirectionalLight()->GetComponent<Light>();
                    DirectX::XMFLOAT3 lightColor = {
                        m_enviromentData.lightColor.r,
                        m_enviromentData.lightColor.g,
                        m_enviromentData.lightColor.b
                    };
                    float lightIntensity = m_enviromentData.lightColor.a;
                    if (ImGui::ColorEdit3("Light Color", &m_enviromentData.lightColor.r)) isChanged = true;
                    if (ImGui::SliderFloat("Intensity", &m_enviromentData.lightColor.a, 0.0f, 5.0f, "%.2f")) isChanged = true;

                    ImGui::Spacing();
                    ImGui::Text("Rotation Angles");
                    if (ImGui::SliderFloat("Pitch", &m_enviromentData.lightPitch, -90.0f, 90.0f, "%.1f deg")) isChanged = true;
                    if (ImGui::SliderFloat("Yaw", &m_enviromentData.lightYaw, 0.0f, 360.0f, "%.1f deg")) isChanged = true;

                    static bool isFirstFrame = true;
                    if (isChanged || isFirstFrame)
                    {
                        auto* mainCamera = m_scene->GetMainCamera()->GetComponent<Camera>();
                        mainCamera->SetClearColor(m_enviromentData.skyColor);

                        // 1. 컴포넌트 포인터 확보
                        if (auto lightObj = m_scene->GetDirectionalLight())
                        {
                            if (auto dirLight = lightObj->GetComponent<Light>())
                            {
                                Color lightColor = Color{
                                    m_enviromentData.lightColor.r * m_enviromentData.lightColor.a,
                                    m_enviromentData.lightColor.g * m_enviromentData.lightColor.a,
                                    m_enviromentData.lightColor.b * m_enviromentData.lightColor.a,
                                    m_enviromentData.lightColor.a
                                };
                                dirLight->SetColor(lightColor);


                                // 3. [방향 벡터 적용] 오일러 -> 쿼터니언 변환 후 즉시 셋업
                                float pitchRad = DirectX::XMConvertToRadians(m_enviromentData.lightPitch);
                                float yawRad = DirectX::XMConvertToRadians(m_enviromentData.lightYaw);

                                DirectX::XMMATRIX rotMatrix = DirectX::XMMatrixRotationRollPitchYaw(pitchRad, yawRad, 0.0f);
                                DirectX::XMVECTOR baseDir = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
                                DirectX::XMVECTOR finalDir = DirectX::XMVector3TransformNormal(baseDir, rotMatrix);
                                finalDir = DirectX::XMVector3Normalize(finalDir);

                                DirectX::XMFLOAT3 finalDirF3;
                                DirectX::XMStoreFloat3(&finalDirF3, finalDir);
                                dirLight->SetDirection(finalDirF3);
                            }
                        }
                        isFirstFrame = false;
                    }
                }
                ImGui::End();
            }

            // 3D 공간 렌더링 (Renderer 레이어)
            m_renderer->Render(m_scene.get());

            m_graphics->BindMainRenderTarget();

            // 4. UI 출력 및 Present
            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            m_graphics->Present();
        }

        // 5. 해제 루틴
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void Sandbox::cameraControll(float dt)
    {
        if (auto mainCamera = m_scene->GetMainCamera())
        {
            auto& input = Input::GetInst();
            auto transform = mainCamera->GetTransform();

            // Camera Pitch, Yaw 때문에 튀는 것을 방지
            static bool isInitialized = false;
            if (!isInitialized)
            {
                DirectX::XMFLOAT3 initialEuler = transform->GetLocalRotationRadians();

                m_cameraPitch = initialEuler.x;
                m_cameraYaw = initialEuler.y;
                isInitialized = true;
            }

            float moveSpeed = 0.001f * dt;
            if (input.KeyPress(DIK_LSHIFT))
                moveSpeed *= BOOST_SPEED;

            float rotSpeed = 0.5f * dt * 0.002f;

            bool isRotated = false;

            if (input.MouseButtonPress(1))
            {
                auto mouseMoveDelta = input.GetMouseMoveDelta();
                if (mouseMoveDelta.x != 0.0f || mouseMoveDelta.y != 0.0f)
                {
                    m_cameraYaw += mouseMoveDelta.x * rotSpeed;
                    m_cameraPitch += mouseMoveDelta.y * rotSpeed;
                    isRotated = true;
                }
            }

            if (input.KeyPress(DIK_LEFT))
            {
                m_cameraYaw -= rotSpeed;
                isRotated = true;
            }
            if (input.KeyPress(DIK_RIGHT))
            {
                m_cameraYaw += rotSpeed;
                isRotated = true;
            }
            if (input.KeyPress(DIK_UP))
            {
                m_cameraPitch -= rotSpeed;
                isRotated = true;
            }
            if (input.KeyPress(DIK_DOWN))
            {
                m_cameraPitch += rotSpeed;
                isRotated = true;
            }

            m_cameraPitch = std::clamp(m_cameraPitch, DirectX::XMConvertToRadians(-89.0f), DirectX::XMConvertToRadians(89.0f));

            if (isRotated)
            {
                DirectX::XMVECTOR cleanRotQuat = DirectX::XMQuaternionRotationRollPitchYaw(m_cameraPitch, m_cameraYaw, 0.0f);
                transform->SetLocalRotationVector(cleanRotQuat);
            }

            DirectX::XMVECTOR forward = transform->GetLocalForwardVector();
            DirectX::XMVECTOR right = transform->GetLocalRightVector();
            DirectX::XMVECTOR up = transform->GetLocalUpVector();

            DirectX::XMVECTOR translation = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

            if (input.KeyPress(DIK_W))
                translation = DirectX::XMVectorAdd(translation, DirectX::XMVectorScale(forward, moveSpeed));
            if (input.KeyPress(DIK_S))
                translation = DirectX::XMVectorSubtract(translation, DirectX::XMVectorScale(forward, moveSpeed));
            if (input.KeyPress(DIK_D))
                translation = DirectX::XMVectorAdd(translation, DirectX::XMVectorScale(right, moveSpeed));
            if (input.KeyPress(DIK_A))
                translation = DirectX::XMVectorSubtract(translation, DirectX::XMVectorScale(right, moveSpeed));
            if (input.KeyPress(DIK_E))
                translation = DirectX::XMVectorAdd(translation, DirectX::XMVectorScale(up, moveSpeed));
            if (input.KeyPress(DIK_Q))
                translation = DirectX::XMVectorSubtract(translation, DirectX::XMVectorScale(up, moveSpeed));

            transform->TranslateVector(translation, eSpace::World);

            {
                auto* dirLight = m_scene->GetDirectionalLight()->GetComponent<Light>();
           
                if (input.KeyDown(DIK_1))
                    dirLight->SetDirection(-1.0f, -1.0f, 1.0f);
                if (input.KeyDown(DIK_2))
                    dirLight->SetDirection(1.0f, -1.0f, 1.0f);
                if (input.KeyDown(DIK_3))
                    dirLight->SetDirection(1.0f, -1.0f, -1.0f);
                if (input.KeyDown(DIK_4))
                    dirLight->SetDirection(-1.0f, -1.0f, -1.0f);
            }
        }
    }
}