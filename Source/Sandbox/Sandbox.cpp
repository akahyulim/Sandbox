#include "pch.h"
#include "Sandbox.h"
#include "Core/Window.h"
#include "Utilities/Timer.h"
#include "Graphics/Graphics.h"
#include "Rendering/Renderer.h"
#include "Input/Input.h"
#include "Scene/Scene.h"
#include "Scene/GameObject.h"
#include "Scene/Components/Camera.h"
#include "Scene/Components/Transform.h"
#include "Scene/Components/MeshRenderer.h"
#include "Scene/Components/Light.h"
#include "Rendering/TextureManager.h"
#include "Rendering/ShaderManager.h"
#include "Rendering/MeshManager.h"
#include "Graphics/RenderTexture.h"

namespace Dive
{
    namespace
    {
        constexpr float BOOST_SPEED = 10.0f;
        constexpr float MIN_SPEED = 0.5f;
        constexpr float MAX_SPEED = 99.0f;
    }

    Sandbox::Sandbox(const SandboxInit& init)
    {
        m_engine = std::make_unique<Engine>(init.engin_init);
        
        m_gui = std::make_unique<ImGuiManager>(m_engine->GetGraphics());

        // logger
        // SetStyle();
        TextureManager::Get().LoadTexture("Assets/Textures/DokeV.jpeg");
        TextureManager::Get().LoadTexture("Assets/Textures/Dmc.jpg");
 
        newScene();
    }

    Sandbox::~Sandbox()
    {
    }

    void Sandbox::Shutdown()
    {
        m_engine->Shutdown();
    }

    void Sandbox::Run()
    {
        // 위치가 여기가 맞나...?
        cameraControll();

        if(m_gui->IsVisible())
        {
            m_engine->Run();
            m_engine->GetGraphics()->ClearBackbuffer();
            m_engine->GetGraphics()->SetBackbuffer();
            
            m_gui->Begin();
            {
                if (!ImGui::IsAnyItemActive())
                {
                    if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Q))
                    {
                        m_engine->GetWindow()->Close();
                    }
                }

                scene();
                menu();
                enviroment();
                hierarchy();
            }
            m_gui->End();

            m_engine->Present();
        }
        else
        {
            m_engine->Run();
            m_engine->Present();
        }
    }

    void Sandbox::OnWindowEvent(const WindowEventData& data)
    {
        //m_engine->OnWindowEvent(data);
        m_gui->HandleWindowMessage(data);
    }
    
    void Sandbox::cameraControll()
    {
        if (m_mainCamera == nullptr)
            return;

        auto dt = Time::GetDeltaTime();

        auto input = m_engine->GetInput();
        auto transform = m_mainCamera->GetTransform();

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
        float rotSpeed = 0.00015f * dt;//0.5f * dt * 0.002f;

        if (input->KeyPress(DIK_LSHIFT))
        {
            moveSpeed *= BOOST_SPEED;
            rotSpeed *= BOOST_SPEED;
        }

        bool isRotated = false;

        if (input->MouseButtonDown(0))
        {
            auto data = m_engine->GetRenderer()->GetPickingData();
            m_selectedObject = m_scene->GetGameObjectByObjectID(data.id);
            //spdlog::info("mouse left button down");
        }

        if (input->MouseButtonPress(1))
        {
            auto mouseMoveDelta = input->GetMouseMoveDelta();
            if (mouseMoveDelta.x != 0.0f || mouseMoveDelta.y != 0.0f)
            {
                m_cameraYaw += mouseMoveDelta.x * rotSpeed;
                m_cameraPitch += mouseMoveDelta.y * rotSpeed;
                isRotated = true;
            }
        }

        if (input->KeyPress(DIK_LEFT))
        {
            m_cameraYaw -= rotSpeed;
            isRotated = true;
        }
        if (input->KeyPress(DIK_RIGHT))
        {
            m_cameraYaw += rotSpeed;
            isRotated = true;
        }
        if (input->KeyPress(DIK_UP))
        {
            m_cameraPitch -= rotSpeed;
            isRotated = true;
        }
        if (input->KeyPress(DIK_DOWN))
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

        if (input->KeyPress(DIK_W))
            translation = DirectX::XMVectorAdd(translation, DirectX::XMVectorScale(forward, moveSpeed));
        if (input->KeyPress(DIK_S))
            translation = DirectX::XMVectorSubtract(translation, DirectX::XMVectorScale(forward, moveSpeed));
        if (input->KeyPress(DIK_D))
            translation = DirectX::XMVectorAdd(translation, DirectX::XMVectorScale(right, moveSpeed));
        if (input->KeyPress(DIK_A))
            translation = DirectX::XMVectorSubtract(translation, DirectX::XMVectorScale(right, moveSpeed));
        if (input->KeyPress(DIK_E))
            translation = DirectX::XMVectorAdd(translation, DirectX::XMVectorScale(up, moveSpeed));
        if (input->KeyPress(DIK_Q))
            translation = DirectX::XMVectorSubtract(translation, DirectX::XMVectorScale(up, moveSpeed));

        transform->TranslateVector(translation, eSpace::World);
    }
    
    void Sandbox::scene()
    {
        auto srv = m_engine->GetRenderer()->GetOffScreenTexture()->GetShaderResourceView();
        ImVec2 screenSize = ImGui::GetIO().DisplaySize;
        ImGui::GetBackgroundDrawList()->AddImage(
            (ImTextureID)srv,
            ImVec2(0, 0),
            screenSize
        );
    }

    void Sandbox::menu()
    {
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
                if (!m_selectedObject)
                {
                    if (ImGui::MenuItem("새로 만들기"))
                    {
                        newScene();
                    }

                    if (ImGui::MenuItem("열기"))
                    {
                        //m_scene->LoadFromFile();
                    }

                    ImGui::Separator();

                    if (ImGui::BeginMenu("3D 오브젝트"))
                    {
                        if (ImGui::MenuItem("트라이앵글"))
                        {
                            auto gameObject = m_scene->CreateGameObject();
                            auto meshRenderer = gameObject->AddComponent<MeshRenderer>();
                            meshRenderer->SetMesh(MeshManager::Get().GetMesh("Triangle"));
                        }
                        if (ImGui::MenuItem("쿼드"))
                        {
                            auto gameObject = m_scene->CreateGameObject();
                            auto meshRenderer = gameObject->AddComponent<MeshRenderer>();
                            meshRenderer->SetMesh(MeshManager::Get().GetMesh("Quad"));
                        }
                        if (ImGui::MenuItem("큐브"))
                        {
                            auto gameObject = m_scene->CreateGameObject();
                            auto meshRenderer = gameObject->AddComponent<MeshRenderer>();
                            meshRenderer->SetMesh(MeshManager::Get().GetMesh("Cube"));
                        }
                        if (ImGui::MenuItem("스피어"))
                        {
                            auto gameObject = m_scene->CreateGameObject();
                            auto meshRenderer = gameObject->AddComponent<MeshRenderer>();
                            meshRenderer->SetMesh(MeshManager::Get().GetMesh("Sphere"));
                        }
                        if (ImGui::MenuItem("캡슐"))
                        {
                            auto gameObject = m_scene->CreateGameObject();
                            auto meshRenderer = gameObject->AddComponent<MeshRenderer>();
                            meshRenderer->SetMesh(MeshManager::Get().GetMesh("Capsule"));
                        }
                        ImGui::EndMenu();
                    }
                    if (ImGui::MenuItem("임포트"))
                    {

                    }

                    ImGui::Separator();

                    if (ImGui::MenuItem("저장"))
                    {
                    }
                    if (ImGui::MenuItem("다른 이름으로 저장"))
                    {
                    }

                    ImGui::Separator();

                    ImGui::MenuItem("계층구조", nullptr, m_windowFlags[Flag_Hierarchy]);
                    ImGui::MenuItem("환경설정", nullptr, &m_showEnvDiralog);

                    ImGui::Separator();

                    if (ImGui::MenuItem("종료", "Ctrl+Q"))
                    {
                        m_engine->GetWindow()->Close();
                    }

                    ImGui::EndPopup();
                }
                else
                {
                    // 여기서부터
                    if(ImGui::MenuItem("복사"))
                    {
                    }
                    if (ImGui::MenuItem("붙어넣기"))
                    {
                    }
                }
            }
        }
        ImGui::End();
    }

    void Sandbox::enviroment()
    {
        if (!m_windowFlags[Flag_Enviroment])
            return;

        /*
        if (ImGui::Begin("환경설정", &m_windowFlags[Flag_Enviroment]))//, ImGuiWindowFlags_NoSavedSettings))
        {
            // 🌟 변경 감지를 위해 하나로 묶기
            bool isChanged = false;

            ImGui::Text("Sky");
            ImGui::Separator();
            m_enviromentData.skyColor = m_mainCamera->GetComponent<Camera>()->GetClearColor();
            if (ImGui::ColorEdit3("Sky Color", &m_enviromentData.skyColor.r)) isChanged = true;

            ImGui::Text("Directional Light");
            ImGui::Separator();

            auto dirLight = m_directionalLight->GetComponent<Light>();
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
                auto* mainCamera = m_mainCamera->GetComponent<Camera>();
                mainCamera->SetClearColor(m_enviromentData.skyColor);

                // 1. 컴포넌트 포인터 확보
                if (auto lightObj = m_directionalLight)
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
        */
    }

    void Sandbox::hierarchy()
    {
        if (!m_windowFlags[Flag_Hierarchy])
            return;
    }

    void Sandbox::newScene()
    {
        m_scene = m_engine->NewScene();
        m_scene->SetName("Sandbox");

        auto& env = m_scene->GetEnviroment();
        env.skyboxCubemap = TextureManager::Get().LoadCubemap(
            //L"Assets/Textures/Skybox/cloudy_skybox.dds");
            //L"Assets/Textures/Skybox/sunsetcube1024.dds");
            L"Assets/Textures/Skybox/desertcube1024.dds");

        m_mainCamera = m_scene->GetCamera();
    }
}