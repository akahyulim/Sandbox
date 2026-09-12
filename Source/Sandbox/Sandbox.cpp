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
#include "Rendering/MaterialManager.h"
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
 
        newScene();
    }

    void Sandbox::Run()
    {
        if(m_gui->IsVisible())
        {
            m_engine->Run();
            m_engine->GetGraphics()->ClearBackbuffer();
            m_engine->GetGraphics()->SetBackbuffer();
            
            m_gui->Begin();
            {
                if (!ImGui::IsAnyItemActive())
                {
                    if (ImGui::IsKeyPressed(ImGuiKey_F1))
                    {
                        m_showEnviromentWindow = !m_showEnviromentWindow;
                    }

                    if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Q))
                    {
                        m_engine->GetWindow()->Close();
                    }
                }

                cameraControll();

                sceneView();
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

    void Sandbox::Shutdown()
    {
        m_engine->Shutdown();
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
            if (m_isSceneViewHovered && !ImGuizmo::IsUsing())
            {
                auto renderer = m_engine->GetRenderer();
                renderer->ProcessPicking();
                auto data = renderer->GetPickingData();
                auto selected = m_scene->GetGameObjectByObjectID(data.id);
                if (m_selectedObject != selected)
                {
                    setSelectedObject(selected);
                }
            }
        }

        if (input->MouseButtonPress(1))
        {
            if (m_isSceneViewHovered)
            {
                auto mouseMoveDelta = input->GetMouseMoveDelta();
                if (mouseMoveDelta.x != 0.0f || mouseMoveDelta.y != 0.0f)
                {
                    m_cameraYaw += mouseMoveDelta.x * rotSpeed;
                    m_cameraPitch += mouseMoveDelta.y * rotSpeed;
                    isRotated = true;
                }
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

    void Sandbox::sceneView()
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();

        static ImVec2 lastWorkSize = ImVec2(0, 0);
        static ImVec2 lastWorkPos = ImVec2(0, 0);

        if (lastWorkSize.x != viewport->WorkSize.x || lastWorkSize.y != viewport->WorkSize.y ||
            lastWorkPos.x != viewport->WorkPos.x || lastWorkPos.y != viewport->WorkPos.y)
        {
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);

            lastWorkSize = viewport->WorkSize;
            lastWorkPos = viewport->WorkPos;
        }

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus;

        if (ImGui::Begin("SceneView", nullptr, windowFlags))
        {
            ImVec2 viewportPos = ImGui::GetCursorScreenPos();
            ImVec2 viewportSize = ImGui::GetContentRegionAvail();

            auto srv = m_engine->GetRenderer()->GetOffScreenTexture()->GetShaderResourceView();
            ImGui::Image((ImTextureID)srv, viewportSize);

            m_isSceneViewHovered = ImGui::IsItemHovered();

            if (ImGui::BeginPopupContextWindow("CanvasContextMenu", ImGuiPopupFlags_MouseButtonRight))
            {
                if (ImGui::MenuItem("New"))
                {
                    newScene();
                }

                if (ImGui::MenuItem("Open"))
                {
                    //m_scene->LoadFromFile();
                }

                ImGui::Separator();

                if (ImGui::BeginMenu("3D Object"))
                {
                    if (ImGui::MenuItem("Triangle"))
                    {
                        auto gameObject = m_scene->CreateGameObject();
                        auto meshRenderer = gameObject->AddComponent<MeshRenderer>();
                        meshRenderer->SetMesh(MeshManager::Get().GetMesh("Triangle"));
                        gameObject->SetName("Triangle");
                    }
                    if (ImGui::MenuItem("Quad"))
                    {
                        auto gameObject = m_scene->CreateGameObject();
                        auto meshRenderer = gameObject->AddComponent<MeshRenderer>();
                        meshRenderer->SetMesh(MeshManager::Get().GetMesh("Quad"));
                        gameObject->SetName("Quad");
                    }
                    if (ImGui::MenuItem("Plane"))
                    {
                        auto gameObject = m_scene->CreateGameObject();
                        auto meshRenderer = gameObject->AddComponent<MeshRenderer>();
                        meshRenderer->SetMesh(MeshManager::Get().GetMesh("Plane"));
                        gameObject->SetName("Plane");
                    }
                    if (ImGui::MenuItem("Cube"))
                    {
                        auto gameObject = m_scene->CreateGameObject();
                        auto meshRenderer = gameObject->AddComponent<MeshRenderer>();
                        meshRenderer->SetMesh(MeshManager::Get().GetMesh("Cube"));
                        gameObject->SetName("Cube");
                    }
                    if (ImGui::MenuItem("Sphere"))
                    {
                        auto gameObject = m_scene->CreateGameObject();
                        auto meshRenderer = gameObject->AddComponent<MeshRenderer>();
                        meshRenderer->SetMesh(MeshManager::Get().GetMesh("Sphere"));
                        gameObject->SetName("Sphere");
                    }
                    if (ImGui::MenuItem("Capsule"))
                    {
                        auto gameObject = m_scene->CreateGameObject();
                        auto meshRenderer = gameObject->AddComponent<MeshRenderer>();
                        meshRenderer->SetMesh(MeshManager::Get().GetMesh("Capsule"));
                        gameObject->SetName("Capsule");
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::MenuItem("Import"))
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

                ImGui::MenuItem("Enviroment", "F1", &m_showEnviromentWindow);

                ImGui::Separator();

                if (ImGui::MenuItem("Quit", "Ctrl+Q"))
                {
                    m_engine->GetWindow()->Close();
                }

                ImGui::EndPopup();
            }

            if (m_selectedObject != nullptr && m_mainCamera != nullptr)
            {
                ImGuizmo::BeginFrame();

                ImGuizmo::SetRect(viewportPos.x, viewportPos.y, viewportSize.x, viewportSize.y);
                ImGuizmo::SetOrthographic(false);
                ImGuizmo::SetDrawlist();

                auto cameraCom = m_mainCamera->GetComponent<Camera>();
                DirectX::XMMATRIX view = cameraCom->GetViewMatrix();
                DirectX::XMMATRIX proj = cameraCom->GetProjectionMatrix();

                auto transform = m_selectedObject->GetTransform();
                DirectX::XMMATRIX world = transform->GetWorldMatrix();

                DirectX::XMFLOAT4X4 viewMat, projMat, worldMat;
                DirectX::XMStoreFloat4x4(&viewMat, view);
                DirectX::XMStoreFloat4x4(&projMat, proj);
                DirectX::XMStoreFloat4x4(&worldMat, world);

                static ImGuizmo::OPERATION currentOperation = ImGuizmo::TRANSLATE;
                static ImGuizmo::MODE currentMode = ImGuizmo::WORLD;

                if (ImGui::IsKeyPressed(ImGuiKey_Z)) currentOperation = ImGuizmo::TRANSLATE;
                if (ImGui::IsKeyPressed(ImGuiKey_X)) currentOperation = ImGuizmo::ROTATE;
                if (ImGui::IsKeyPressed(ImGuiKey_C)) currentOperation = ImGuizmo::SCALE;

                if (ImGuizmo::Manipulate(&viewMat._11, &projMat._11, currentOperation, currentMode, &worldMat._11))
                {
                    DirectX::XMMATRIX newWorld = DirectX::XMLoadFloat4x4(&worldMat);
                    transform->SetWorldMatrix(newWorld);
                }
            }
        }   

        ImGui::End();

        showEnviroment();
    }

    void Sandbox::showEnviroment()
    {
        if (!m_showEnviromentWindow)
            return;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos({ viewport->WorkSize.x - 350.0f, viewport->WorkPos.y });
        ImGui::SetNextWindowSize({ 350.0f, viewport->WorkSize.y });

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoTitleBar | 
            ImGuiWindowFlags_HorizontalScrollbar;

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));

        if (ImGui::Begin("Environment", &m_showEnviromentWindow, flags))
        {
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::CollapsingHeader("Sky"))
            {
                auto renderer = m_engine->GetRenderer();

                static int e = 0;
                ImGui::RadioButton("Skybox", &e, 0); ImGui::SameLine();
                ImGui::RadioButton("Uniform Color", &e, 1);

                renderer->SetSkyMode(static_cast<eSkyMode>(e));

                if (e == 0)
                {
                    const char* items[] = { "Cloudy", "Sunset", "Desert"};
                    static int item_current = 0;
                    ImGui::Combo("cube map", &item_current, items, IM_COUNTOF(items));

                    auto handle = m_skyCubemaps[items[item_current]];
                    m_scene->GetEnviroment().skyboxCubemap = handle;    // 이 부분이 마음에 들지 않는다.
                }
                else
                {
                    auto skyColor = renderer->GetSkyColor();
                    ImGui::ColorEdit3("uniform color", (float*)&skyColor);
                    renderer->SetSkyColor(skyColor);
                }
            }

            ImGui::Separator();
        }
        ImGui::End();
        ImGui::PopStyleColor();
    }

    void Sandbox::newScene()
    {
        m_scene = m_engine->NewScene();
        m_scene->SetName("Sandbox");

        m_skyCubemaps.emplace("Cloudy", TextureManager::Get().LoadCubemap(L"Assets/Textures/Skybox/cloudy_skybox.dds"));
        m_skyCubemaps.emplace("Sunset", TextureManager::Get().LoadCubemap(L"Assets/Textures/Skybox/sunsetcube1024.dds"));
        m_skyCubemaps.emplace("Desert", TextureManager::Get().LoadCubemap(L"Assets/Textures/Skybox/desertcube1024.dds"));

        auto& env = m_scene->GetEnviroment();
        env.skyboxCubemap = m_skyCubemaps["Cloudy"];

        m_mainCamera = m_scene->GetCamera();

        auto bottom = m_scene->CreateGameObject();
        bottom->SetName("Bottom");
        auto meshRenderer = bottom->AddComponent<MeshRenderer>();
        meshRenderer->SetMesh(MeshManager::Get().GetMesh("Plane"));

        auto mtrl = MaterialManager::Get().CreateMaterial("Bottom");
        mtrl->SetMap("Assets/Textures/stone01.tga", eMapType::Albedo);
        mtrl->SetMap("Assets/Textures/normal01.tga", eMapType::Normal);

        meshRenderer->SetMaterial(mtrl);
    }

    void Sandbox::setSelectedObject(GameObject* obj)
    {
        if (m_selectedObject != obj)
        {
            m_selectedObject = obj;

            // ID 추출 및 렌더러 동기화를 이 안에서 한 번에 처리
            uint32_t id = (obj != nullptr) ? obj->GetComponent<MeshRenderer>()->GetObjectID() : 0; // 엔진 설계에 맞게 ID 취득
            m_engine->GetRenderer()->SetSelectedObjectID(id);
        }
    }
}