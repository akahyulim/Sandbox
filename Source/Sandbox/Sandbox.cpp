#include "pch.h"
#include "Sandbox.h"
#include "ImGuiHelper.h"
#include "Core/Window.h"
#include "Utilities/Timer.h"
#include "Utilities/FileUtils.h"
#include "Graphics/Graphics.h"
#include "Rendering/Renderer.h"
#include "Rendering/TextureManager.h"
#include "Rendering/ShaderManager.h"
#include "Rendering/Mesh.h"
#include "Rendering/MeshManager.h"
#include "Rendering/Material.h"
#include "Rendering/MaterialManager.h"
#include "Graphics/RenderTexture.h"
#include "Input/Input.h"
#include "Scene/Scene.h"
#include "Scene/GameObject.h"
#include "Scene/Components/Camera.h"
#include "Scene/Components/Transform.h"
#include "Scene/Components/MeshRenderer.h"
#include "Scene/Components/Light.h"
#include "Scene/SceneSerializer.h"

namespace Dive
{
    namespace
    {
        constexpr float BOOST_SPEED = 10.0f;
        constexpr float MIN_SPEED = 0.5f;
        constexpr float MAX_SPEED = 99.0f;
        constexpr float LIGHT_ICON_SIZE = 64.0f;
    }

    Sandbox::Sandbox(const SandboxInit& init)
    {
        m_engine = std::make_unique<Engine>(init.engin_init);
        m_gui = std::make_unique<ImGuiManager>(m_engine->GetGraphics());

        loadResources();
 
        //sceneInnocent();
        sceneLighting();
       
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
                    if (ImGui::IsKeyPressed(ImGuiKey_Escape))
                    {
                        if (m_showEnvironmentMenu) m_showEnvironmentMenu = false;
                        else if (m_showInspectorMenu) m_showInspectorMenu = false;
                        else if (m_selectedMaterial) m_selectedMaterial = nullptr; 
                        else if (m_selectedObject) setSelectedObject(nullptr);
                        else m_showOptionMenu = !m_showOptionMenu;
                    }

                    if (ImGui::IsKeyPressed(ImGuiKey_F1))
                    {
                        m_showEnvironmentMenu = !m_showEnvironmentMenu;
                        m_showInspectorMenu = false;
                    }

                    if (ImGui::IsKeyPressed(ImGuiKey_I) && m_selectedObject)
                    {
                        m_showEnvironmentMenu = false;
                        m_showInspectorMenu = !m_showInspectorMenu;
                        if (m_showInspectorMenu) m_inspectorMode = eInspectorMode::ShowComponents;
                        
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

       //if (ImGui::GetIO().WantCaptureKeyboard)
       //    return;

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
        float rotSpeed = 0.0001f * dt;

        if (input->KeyPress(DIK_LSHIFT))
        {
            moveSpeed *= BOOST_SPEED;
            rotSpeed *= BOOST_SPEED;
        }

        bool isRotated = false;

        /*
        // 현재 기능이 중복된다.
        // 차라리 ctrl 등이랑 같이 누르는 방향으로 가자.
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
        */
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

            renderLightIcons(viewportPos, viewportSize);

            if (m_isSceneViewHovered && 
                !ImGuizmo::IsUsing() && !ImGuizmo::IsOver() 
                && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                handleSceneClick(viewportPos, viewportSize);
            }

            if (//m_isSceneViewHovered && !ImGuizmo::IsUsing() && !ImGuizmo::IsOver() && 
                ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            {
                // 현재 Light는 반응하지 않는다.
                // 아마도 Selected와 ContextTarget이 일치하지 않기 때문인 듯하다.
                // Light는 MeshRenderer가 없어서 Picking을 다르게 하고 있다.
                auto renderer = m_engine->GetRenderer();
                renderer->ProcessPicking();
                auto data = renderer->GetPickingData();

                m_contextTargetObject = m_scene->GetGameObjectByObjectID(data.id);
                
                ImGui::OpenPopup("SceneViewContextMenu");
            }
            drawCanvasContextMenu();

            renderGizmo(viewportPos, viewportSize);
        }   

        ImGui::End();

        showEnvironment();
        showInspector();
        showOption();
    }

    void Sandbox::renderLightIcons(const ImVec2& viewportPos, const ImVec2& viewportSize)
    {
        if (m_mainCamera != nullptr && !m_scene->GetLightQueue().empty())
        {
            auto cameraCom = m_mainCamera->GetComponent<Camera>();
            DirectX::XMMATRIX viewProj = cameraCom->GetViewProjMatrix();
            ImDrawList* drawList = ImGui::GetWindowDrawList();

            for (const auto& lightObject : m_scene->GetLightQueue())
            {
                auto type = lightObject->GetComponent<Light>()->GetLightType();
                DirectX::XMVECTOR worldPos = lightObject->GetTransform()->GetPositionVector();

                float screenX, screenY;
                if (!ImGuiHelper::WorldToScreenPoint(worldPos, viewProj, viewportPos, viewportSize, screenX, screenY))
                    continue;

                auto textureView = (type == eLightType::Point) ?
                    TextureManager::Get().GetTextureView(m_pointLightIcon) :
                    TextureManager::Get().GetTextureView(m_spotLightIcon);

                drawList->AddImage((ImTextureID)textureView,
                    ImVec2(screenX - LIGHT_ICON_SIZE * 0.5f, screenY - LIGHT_ICON_SIZE * 0.5f),
                    ImVec2(screenX + LIGHT_ICON_SIZE * 0.5f, screenY + LIGHT_ICON_SIZE * 0.5f));
            }
        }
    }

    void Sandbox::drawCanvasContextMenu()
    {
        if (ImGui::BeginPopup("SceneViewContextMenu"))
        {
            if(m_selectedObject && m_contextTargetObject != nullptr && m_selectedObject == m_contextTargetObject)
            {
                ImGui::MenuItem("Edit", "I", &m_showInspectorMenu);

                ImGui::Separator();

                if (ImGui::MenuItem("Cut"))
                {

                }

                if (ImGui::MenuItem("Duplicate"))
                {
                    // 복제 로직
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Delete"))
                {
                    m_scene->RemoveGameObject(m_contextTargetObject);
                    setSelectedObject(nullptr);
                }
            }
            else
            {
                if (ImGui::MenuItem("Environment"))
                {
                    m_showEnvironmentMenu = true;
                    m_showInspectorMenu = false;
                }

                if (ImGui::BeginMenu("Add"))
                {
                    if (ImGui::BeginMenu("Light"))
                    {
                        if (ImGui::MenuItem("Point Light"))
                        {
                            createPointLight();
                        }
                        if (ImGui::MenuItem("Spot Light"))
                        {
                            createSpotLight();
                        }
                        ImGui::EndMenu();
                    }

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

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Create"))
                {
                    if (ImGui::MenuItem("Material"))
                    {
                        m_showInspectorMenu = true;
                        m_inspectorMode = eInspectorMode::NewMaterial;
                    }

                    ImGui::EndMenu();
                }
            }

            ImGui::EndPopup();
        }
    }

    void Sandbox::handleSceneClick(const ImVec2& viewportPos, const ImVec2& viewportSize)
    {
        bool clickedLightIcon = false;

        if (m_mainCamera != nullptr && !m_scene->GetLightQueue().empty())
        {
            auto cameraCom = m_mainCamera->GetComponent<Camera>();
            DirectX::XMMATRIX viewProj = cameraCom->GetViewProjMatrix();

            ImVec2 mousePos = ImGui::GetMousePos();
            float iconSize = 64.0f;
            float halfSize = iconSize * 0.5f;

            for (const auto& lightObject : m_scene->GetLightQueue())
            {
                DirectX::XMVECTOR worldPos = lightObject->GetTransform()->GetPositionVector();

                float screenX, screenY;
                if (!ImGuiHelper::WorldToScreenPoint(worldPos, viewProj, viewportPos, viewportSize, screenX, screenY))
                    continue;

                ImVec2 minBound(screenX - halfSize, screenY - halfSize);
                ImVec2 maxBound(screenX + halfSize, screenY + halfSize);

                // 마우스가 라이트 아이콘 영역 안에 있는지 확인
                if (mousePos.x >= minBound.x && mousePos.x <= maxBound.x &&
                    mousePos.y >= minBound.y && mousePos.y <= maxBound.y)
                {
                    if (m_selectedObject != lightObject)
                    {
                        m_selectedObject = lightObject;
                        m_showInspectorMenu = false;
                    }
                    clickedLightIcon = true;
                    break;
                }
            }
        }

        // 라이트 아이콘을 클릭하지 않았다면 일반 3D 메쉬 픽킹 수행
        if (!clickedLightIcon)
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

    void Sandbox::renderGizmo(const ImVec2& viewportPos, const ImVec2& viewportSize)
    {
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

    void Sandbox::showEnvironment()
    {
        if (!m_showEnvironmentMenu)
            return;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos({ viewport->WorkSize.x - 370.0f, viewport->WorkPos.y });
        ImGui::SetNextWindowSize({ 370.0f, viewport->WorkSize.y });

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_HorizontalScrollbar;

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, 0.75f));

        if (ImGui::Begin("Environment", &m_showEnvironmentMenu, flags))
        {
            if (ImGui::CollapsingHeader("Sky", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto renderer = m_engine->GetRenderer();

                int currentSkyMode = static_cast<int>(renderer->GetSkyMode());

                // Skybox랑 Sphere를 하나로 합치고 싶다.
                if (ImGui::RadioButton("Skybox", &currentSkyMode, 0))
                    renderer->SetSkyMode(eSkyMode::Skybox);
                ImGui::SameLine();
                if (ImGui::RadioButton("SkySphere", &currentSkyMode, 1))
                    renderer->SetSkyMode(eSkyMode::SkySphere);
                ImGui::SameLine();
                if(ImGui::RadioButton("Uniform Color", &currentSkyMode, 2))
                    renderer->SetSkyMode(eSkyMode::UniformColor);

                if (currentSkyMode == 0)
                {
                    const char* items[] = { "Cloudy", "Sunset", "Desert"};
                    static int item_current = 0;
                    ImGui::Combo("##Cube Map", &item_current, items, IM_COUNTOF(items));

                    auto handle = m_skyCubemaps[items[item_current]];
                    m_scene->GetEnviroment().skyboxCubemap = handle;    // 이 부분이 마음에 들지 않는다.
                }
                else if (currentSkyMode == 1)
                {
                    const char* items[] = { "night_puresky", "dawn", "clear_night"};
                    static int item_current = 0; 
                    ImGui::Combo("##Cube Map", &item_current, items, IM_COUNTOF(items));

                    auto handle = m_skyCubemaps[items[item_current]];
                    m_scene->GetEnviroment().skyboxCubemap = handle;    // 이 부분이 마음에 들지 않는다.
                }
                else
                {
                    // Apex Color
                    {
                        ImGui::PushID("ApexColor");
                        auto color = renderer->GetApxeColor();
                        ImGui::Columns(2);
                        ImGui::SetColumnWidth(0, 100.0f);
                        ImGui::Text("Apex");
                        ImGui::NextColumn();
                        ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoInputs;
                        ImGui::ColorEdit3("##ApexColor", (float*)&color, flags);
                        renderer->SetApexColor(color);
                        ImGui::Columns(1);
                        ImGui::PopID();
                    }

                    // Horizon Color
                    {
                        ImGui::PushID("HorizonColor");
                        auto color = renderer->GetHorizonColor();
                        ImGui::Columns(2);
                        ImGui::SetColumnWidth(0, 100.0f);
                        ImGui::Text("Horizon");
                        ImGui::NextColumn();
                        ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoInputs;
                        ImGui::ColorEdit3("##HorizonColor", (float*)&color, flags);
                        renderer->SetHorizonColor(color);
                        ImGui::Columns(1);
                        ImGui::PopID();
                    }
                }
            }

            ImGui::Separator();

            // 현재 dir light가 Renderer에 귀속되어 있다.
            // weather 구현 중 같이 묶인 것 같다.
            if (ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto renderer = m_engine->GetRenderer();

                // direction
                // -1.0f ~ 1.0 사이로 제한해야 한다.
                // 그림판3D처럼 제어하고 싶다.
                {
                    DirectX::XMFLOAT3 dir = renderer->GetLightDir();
                    ImGuiHelper::DrawVec3Control("Direction", dir, 0.0f, 100.0f);
                    renderer->SetLightDir(dir);
                }

                // color
                {
                    ImGui::PushID("Color");
                    auto color = renderer->GetLightColor();
                    ImGui::Columns(2);
                    ImGui::SetColumnWidth(0, 100.0f);
                    ImGui::Text("Color");
                    ImGui::NextColumn();
                    ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoInputs;
                    ImGui::ColorEdit3("##LightColor", (float*)&color, flags);
                    renderer->SetLightColor(color);
                    ImGui::Columns(1);
                    ImGui::PopID();
                }

                // ambient
                {
                    ImGui::PushID("Ambient");
                    auto color = renderer->GetAmbientColor();
                    ImGui::Columns(2);
                    ImGui::SetColumnWidth(0, 100.0f);
                    ImGui::Text("Ambient");
                    ImGui::NextColumn();
                    ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoInputs;
                    ImGui::ColorEdit3("##AmbientColor", (float*)&color, flags);
                    renderer->SetAmbientColor(color);
                    ImGui::Columns(1);
                    ImGui::PopID();
                }
            }

            /*
            ImGui::Separator();
            
            if (ImGui::CollapsingHeader("EFFECT", ImGuiTreeNodeFlags_DefaultOpen))
            {
                // smog
                // rain
            }
            */
        }
        ImGui::End();
        ImGui::PopStyleColor();
    }

    void Sandbox::showInspector()
    {
        if (!m_showInspectorMenu)
            return;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos({ viewport->WorkSize.x - 370.0f, viewport->WorkPos.y });
        ImGui::SetNextWindowSize({ 370.0f, viewport->WorkSize.y });

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_HorizontalScrollbar;

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, 0.75f));

        if (ImGui::Begin("Inspector", &m_showInspectorMenu, ImGuiWindowFlags_NoCollapse))
        {
            switch (m_inspectorMode)
            {
            case eInspectorMode::NewMaterial:
            {
                showNewMaterial();
                break;
            }
            case eInspectorMode::ShowComponents:
            {
                showComponents();
                break;
            }
            case eInspectorMode::ShowMaterial:
            {
                showMaterial();
                break;
            }
            }
        }

        ImGui::End();
        ImGui::PopStyleColor();
    }

    // ShowOption 쪽이 더 어울린다.
    void Sandbox::showOption()
    {
        if (!m_showOptionMenu)
            return;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();

        // 원본 크기의 1.5배로 설정 (너비 240, 높이 261)
        ImVec2 windowSize = { 240.0f, 261.0f };
        ImGui::SetNextWindowSize(windowSize);
        ImGui::SetNextWindowPos(
            {
                viewport->WorkPos.x + (viewport->WorkSize.x - windowSize.x) * 0.5f,
                viewport->WorkPos.y + (viewport->WorkSize.y - windowSize.y) * 0.5f
            },
            ImGuiCond_Always
        );

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoScrollbar;

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));

        // 패딩 및 간격 1.5배 적용 (Padding: 15.0f, Spacing: 9.0f)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0f, 15.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 9.0f));

        if (ImGui::Begin("Quit", &m_showOptionMenu, flags))
        {
            // 버튼 크기 1.5배 적용 (135x26 -> 202.5x39)
            float buttonWidth = 202.5f;
            float buttonHeight = 39.0f;
            float windowWidth = ImGui::GetWindowSize().x;

            ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
            if (ImGui::Button("New", ImVec2(buttonWidth, buttonHeight)))
            {
                sceneInnocent();
                m_showOptionMenu = false;
            }

            // 2. Save
            ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
            if (ImGui::Button("Save", ImVec2(buttonWidth, buttonHeight)))
            {
                // TODO: 저장 로직

                MaterialManager::Get().SaveToFiles();

                SceneSerializer serializer(m_scene);
                serializer.Serialize("Assets/Scenes");
            }

            // 3. Load
            ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
            if (ImGui::Button("Load", ImVec2(buttonWidth, buttonHeight)))
            {
                sceneLighting();
                m_showOptionMenu = false;
            }

            // 4. Option
            ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
            if (ImGui::Button("Options", ImVec2(buttonWidth, buttonHeight)))
            {
                //m_showOptionMenu = false;
                //m_showEnvironmentMenu = true;
            }

            // 5. Quit
            ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
            if (ImGui::Button("Quit", ImVec2(buttonWidth, buttonHeight)))
            {
                m_engine->GetWindow()->Close();
            }
        }
        ImGui::End();

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
    }

    void Sandbox::showComponents()
    {
        if (!m_selectedObject)
            return;

        // Active
        {
            bool isActive = m_selectedObject->IsActive();
            if (ImGui::Checkbox("##Active", &isActive))
            {
                m_selectedObject->SetActive(isActive);
            }
        }

        ImGui::SameLine();

        // Name
        {
            std::string name = m_selectedObject->GetName();
            if (ImGui::InputText("##Name", &name))
            {
                m_selectedObject->SetName(name);
            }
        }

        ImGui::Separator();

        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            auto transform = m_selectedObject->GetTransform();

            auto position = transform->GetLocalPosition();
            ImGuiHelper::DrawVec3Control("Position", position, 0.0f, 100.0f);
            transform->SetLocalPosition(position);

            auto rotation = transform->GetLocalRotationDegrees();
            ImGuiHelper::DrawVec3Control("Rotation", rotation, 0.0f, 100.0f);
            transform->SetLocalRotationByDegrees(rotation);

            auto scale = transform->GetLocalScale();
            ImGuiHelper::DrawVec3Control("Scale", scale, 1.0f, 100.0f);
            transform->SetLocalScale(scale);
        }

        ImGui::Separator();

        if (m_selectedObject->HasComponent<Light>())
        {
            if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto light = m_selectedObject->GetComponent<Light>();

                // type
                ImGui::PushID("LightType");
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, 100.0f);
                ImGui::Text("Type");
                ImGui::NextColumn();
                std::vector<const char*> lightTypes;
                lightTypes.push_back("Point");
                lightTypes.push_back("Spot");
                int currentType = static_cast<int>(light->GetLightType());
                ImGui::Combo("##LightType", &currentType, lightTypes.data(), static_cast<int>(lightTypes.size()));
                light->SetLightType(static_cast<eLightType>(currentType));
                ImGui::Columns(1);
                ImGui::PopID();

                // range
                ImGui::PushID("LightRange");
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, 100.0f);
                ImGui::Text("Range");
                ImGui::NextColumn();
                float range = light->GetRange();
                ImGui::DragFloat("##LightRange", &range, 0.1f, 0.0f, 0.0f, "%.2f");
                light->SetRange(range);
                ImGui::Columns(1);
                ImGui::PopID();

                // color
                ImGui::PushID("LightColor");
                ImVec4 color = { light->GetColor().x, light->GetColor().y, light->GetColor().z, 1.0f };
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, 100.0f);
                ImGui::Text("Color");
                ImGui::NextColumn();
                ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoInputs;
                ImGui::ColorEdit3("##LightColor", (float*)&color, flags);
                light->SetColor(color.x, color.y, color.z);
                ImGui::Columns(1);
                ImGui::PopID();

                // intensity
                ImGui::PushID("LightIntensity");
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, 100.0f);
                ImGui::Text("Intensity");
                ImGui::NextColumn();
                float intensity = light->GetIntensity();
                ImGui::DragFloat("##LightIntensity", &intensity, 0.1f, 0.0f, 8.0f, "%.2f");
                light->SetIntensity(intensity);
                ImGui::Columns(1);
                ImGui::PopID();

                if (currentType == static_cast<int>(eLightType::Spot))
                {
                    // Inner Angle
                    ImGui::PushID("LightInnerAngle");
                    ImGui::Columns(2);
                    ImGui::SetColumnWidth(0, 100.0f);
                    ImGui::Text("Inner Angle");
                    ImGui::NextColumn();
                    float innerAngle = light->GetInnerAngleDegrees();
                    if (ImGui::DragFloat("##LightInnerAngle", &innerAngle, 0.5f, 0.0f, 90.0f, "%.1f"))
                    {
                        light->SetInnerAngleDegrees(innerAngle);
                    }
                    ImGui::Columns(1);
                    ImGui::PopID();

                    // Outer Angle
                    ImGui::PushID("LightOuterAngle");
                    ImGui::Columns(2);
                    ImGui::SetColumnWidth(0, 100.0f);
                    ImGui::Text("Outer Angle");
                    ImGui::NextColumn();
                    float outerAngle = light->GetOuterAngleDegrees();
                    if (ImGui::DragFloat("##LightOuterAngle", &outerAngle, 0.5f, 0.0f, 90.0f, "%.1f"))
                    {
                        light->SetOuterAngleDegrees(outerAngle);
                    }
                    ImGui::Columns(1);
                    ImGui::PopID();
                }
            }
        }

        ImGui::Separator();

        if (m_selectedObject->HasComponent<MeshRenderer>())
        {
            auto meshRenderer = m_selectedObject->GetComponent<MeshRenderer>();

            if (ImGui::CollapsingHeader("Mesh Renderer", ImGuiTreeNodeFlags_DefaultOpen))
            {
                {
                    auto mesh = meshRenderer->GetMesh();

                    // 1. Mesh 이름 표시 영역 (더블클릭 시 반응)
                    ImGui::PushID("MeshName");
                    std::string meshName = (mesh != nullptr) ? "mesh->GetName()" : "No Mesh"; // 실제 구현에 맞게 수정

                    ImGui::Columns(2);
                    ImGui::SetColumnWidth(0, 100.0f);
                    ImGui::Text("Mesh");
                    ImGui::NextColumn();

                    float spacing = ImGui::GetStyle().ItemSpacing.x;
                    float buttonWidth = ImGui::GetContentRegionAvail().x - 30.0f - spacing; // '...' 버튼 공간 확보

                    // 입력창 느낌을 주는 버튼 혹은 커스텀 드로잉 영역
                    // 여기서는 깔끔하게 전체 너비를 쓰는 버튼 형태나 Selectable을 활용할 수 있습니다.
                    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));

                    // 버튼 크기를 컬럼 크기에 맞게 꽉 채우기 위해 -1.0f 사용
                    if (ImGui::Button(meshName.c_str(), ImVec2(buttonWidth, 0.0f)))
                    {
                        // 일반 클릭 시 처리할 내용이 있다면 여기에 작성
                    }

                    ImGui::PopStyleColor(3);

                    ImGui::SameLine();
                    if (ImGui::Button("◎##Mesh", ImVec2(30.0f, 0.0f)))
                    {
                    }

                    ImGui::Columns(1);
                    ImGui::PopID();
                }

                {
                    auto material = meshRenderer->GetMaterial();

                    ImGui::PushID("MaterialName");
                    std::string mtrlName = (material != nullptr) ? material->GetName() : "No Material";

                    ImGui::Columns(2);
                    ImGui::SetColumnWidth(0, 100.0f);
                    ImGui::Text("Material");
                    ImGui::NextColumn();

                    // 버튼의 남은 공간(오른쪽에 '...' 버튼을 두기 위함) 계산을 위해 너비 조절 가능
                    // 버튼과 '...' 버튼을 같은 줄에 배치하기 위한 처리
                    float spacing = ImGui::GetStyle().ItemSpacing.x;
                    float buttonWidth = ImGui::GetContentRegionAvail().x - 30.0f - spacing; // '...' 버튼 공간 확보

                    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));

                    if (ImGui::Button(mtrlName.c_str(), ImVec2(buttonWidth, 0.0f)))
                    {
                        m_selectedMaterial = material;
                        m_inspectorMode = eInspectorMode::ShowMaterial;
                    }

                    ImGui::PopStyleColor(3);

                    ImGui::SameLine();
                    if (ImGui::Button("◎##Material", ImVec2(30.0f, 0.0f)))
                    {
                        ImGui::OpenPopup("Materials");
                    }

                    popupMaterials();

                    ImGui::Columns(1);
                    ImGui::PopID();
                }
            }
        }
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

        m_selectedMaterial = nullptr;

        if(m_selectedObject == nullptr) m_showInspectorMenu = false;
    }

    void Sandbox::loadResources()
    {
        // sky
        {
            m_skyCubemaps.emplace("Cloudy", TextureManager::Get().LoadCubemap(L"Assets/Textures/Skybox/cloudy_skybox.dds"));
            m_skyCubemaps.emplace("Sunset", TextureManager::Get().LoadCubemap(L"Assets/Textures/Skybox/sunsetcube1024.dds"));
            m_skyCubemaps.emplace("Desert", TextureManager::Get().LoadCubemap(L"Assets/Textures/Skybox/desertcube1024.dds"));

            m_skyCubemaps.emplace("night_puresky", TextureManager::Get().LoadTexture(L"Assets/Textures/Skysphere/qwantani_night_puresky_4k.hdr"));
            m_skyCubemaps.emplace("dawn", TextureManager::Get().LoadTexture(L"Assets/Textures/Skysphere/aarfontein_dawn_2_4k.hdr"));
            m_skyCubemaps.emplace("clear_night", TextureManager::Get().LoadTexture(L"Assets/Textures/Skysphere/rogland_clear_night_4k.hdr"));
        }

        // icons
        {
            m_pointLightIcon = TextureManager::Get().LoadTexture(L"Assets/Textures/Sandbox/pointlight.png");
            m_spotLightIcon = TextureManager::Get().LoadTexture(L"Assets/Textures/Sandbox/spotlight.png");
        }

        // materials
        {
            MaterialManager::Get().LoadFromFile("Assets/Materials/Tiles106");
            MaterialManager::Get().LoadFromFile("Assets/Materials/Metal_Plate");
            MaterialManager::Get().LoadFromFile("Assets/Materials/Rusty_Metal_Grid");
            MaterialManager::Get().LoadFromFile("Assets/Materials/Stacked_Brick_Wall");
            MaterialManager::Get().LoadFromFile("Assets/Materials/Marble_Cliff_06");
            MaterialManager::Get().LoadFromFile("Assets/Materials/Rust_Metal_05");
            MaterialManager::Get().LoadFromFile("Assets/Materials/No_Texture");
            MaterialManager::Get().LoadFromFile("Assets/Materials/White");
            MaterialManager::Get().LoadFromFile("Assets/Materials/Metal White");
            MaterialManager::Get().LoadFromFile("Assets/Materials/Gray");
            MaterialManager::Get().LoadFromFile("Assets/Materials/Orange");
            MaterialManager::Get().LoadFromFile("Assets/Materials/Brown");
            MaterialManager::Get().LoadFromFile("Assets/Materials/Mint");
        }
    }

    void Sandbox::sceneEmpty()
    {
        m_scene = m_engine->NewScene();
        m_scene->SetName("Empty");
    }

    void Sandbox::sceneInnocent()
    {
        m_scene = m_engine->NewScene();

        auto renderer = m_engine->GetRenderer();
        renderer->SetSkyMode(eSkyMode::UniformColor);
        renderer->SetApexColor({ 1.0f, 1.0f, 1.0f, 1.0f });

        // camera
        {
            m_mainCamera = m_scene->GetCamera();
            auto transform = m_mainCamera->GetTransform();
            transform->SetLocalPosition(0.0f, 5.0f, -12.0f);
            transform->SetRotationByDegrees({ 15.0f, 0.0f, 0.0f });
        }
        SceneSerializer serializer(m_scene);
        serializer.Deserialize("Assets/Scenes/Innocent.scene");
        /*
        m_scene = m_engine->NewScene();
        m_scene->SetName("Innocent");

        auto renderer = m_engine->GetRenderer();
        renderer->SetSkyMode(eSkyMode::UniformColor);
        renderer->SetApexColor({ 1.0f, 1.0f, 1.0f, 1.0f });

        // camera
        {
            m_mainCamera = m_scene->GetCamera();
            auto transform = m_mainCamera->GetTransform();
            transform->SetLocalPosition(0.0f, 5.0f, -12.0f);
            transform->SetRotationByDegrees({ 15.0f, 0.0f, 0.0f });
        }

        // Lights
        {
            // Dir Light
            {
                renderer->SetLightDir(1.0f, -1.0f, 1.0f);
            }
        }

        // Objects
        {
            // bottom
            {
                auto bottom = m_scene->CreateGameObject();
                bottom->SetName("Bottom");
                auto meshRenderer = bottom->AddComponent<MeshRenderer>();
                meshRenderer->SetMesh(MeshManager::Get().GetMesh("Plane"));

                auto mtrl = MaterialManager::Get().GetMaterial("Assets/Materials/Gray");
                meshRenderer->SetMaterial(mtrl);
            }


            // Brown Cube
            {
                auto cube = m_scene->CreateGameObject();
                auto meshRenderer = cube->AddComponent<MeshRenderer>();
                meshRenderer->SetMesh(MeshManager::Get().GetMesh("Cube"));
                cube->SetName("Brown Cube");

                auto mtrl = MaterialManager::Get().GetMaterial("Assets/Materials/Brown");
                meshRenderer->SetMaterial(mtrl);

                auto transform = cube->GetTransform();
                transform->SetLocalPosition(-1.0f, 0.5f, 1.5f);
                transform->SetLocalRotationByDegrees(0.0f, 45.0f, 0.0f);
            }

            // Mint Cube
            {
                auto cube = m_scene->CreateGameObject();
                auto meshRenderer = cube->AddComponent<MeshRenderer>();
                meshRenderer->SetMesh(MeshManager::Get().GetMesh("Cube"));
                cube->SetName("Mint Cube");

                auto mtrl = MaterialManager::Get().GetMaterial("Assets/Materials/Mint");
                meshRenderer->SetMaterial(mtrl);

                auto transform = cube->GetTransform();
                transform->SetLocalPosition(1.0f, 1.0f, 0.0f);
                transform->SetScale(1.0f, 2.0f, 1.0f);
            }

            // Metal White Spherer
            {
                auto sphere = m_scene->CreateGameObject();
                auto meshRenderer = sphere->AddComponent<MeshRenderer>();
                meshRenderer->SetMesh(MeshManager::Get().GetMesh("Sphere"));
                sphere->SetName("Metal White Sphere");

                auto mtrl = MaterialManager::Get().GetMaterial("Assets/Materials/Metal White");
                meshRenderer->SetMaterial(mtrl);

                auto transform = sphere->GetTransform();
                transform->SetLocalPosition(-3.0f, 0.5f, -4.0f);
            }

            // White Spherer
            {
                auto sphere = m_scene->CreateGameObject();
                auto meshRenderer = sphere->AddComponent<MeshRenderer>();
                meshRenderer->SetMesh(MeshManager::Get().GetMesh("Sphere"));
                sphere->SetName("White Sphere");

                auto mtrl = MaterialManager::Get().GetMaterial("Assets/Materials/White");
                meshRenderer->SetMaterial(mtrl);

                auto transform = sphere->GetTransform();
                transform->SetLocalPosition(-1.5f, 0.5f, -4.0f);
            }

            // White Spherer
            {
                auto sphere = m_scene->CreateGameObject();
                auto meshRenderer = sphere->AddComponent<MeshRenderer>();
                meshRenderer->SetMesh(MeshManager::Get().GetMesh("Sphere"));
                sphere->SetName("White Sphere");

                auto mtrl = MaterialManager::Get().GetMaterial("Assets/Materials/White");
                meshRenderer->SetMaterial(mtrl);

                auto transform = sphere->GetTransform();
                transform->SetLocalPosition(0.0f, 0.5f, -4.0f);
            }

            // Orange Triangle
            {
                auto triangle = m_scene->CreateGameObject();
                auto meshRenderer = triangle->AddComponent<MeshRenderer>();
                meshRenderer->SetMesh(MeshManager::Get().GetMesh("Triangle"));
                triangle->SetName("Orange Triangle");

                auto mtrl = MaterialManager::Get().GetMaterial("Assets/Materials/Orange");
                meshRenderer->SetMaterial(mtrl);

                auto transform = triangle->GetTransform();
                transform->SetLocalPosition(1.5f, 0.5f, -2.0f);
            }
        }
        */
    }

    void Sandbox::sceneLighting()
    {
        m_scene = m_engine->NewScene();
        auto& env = m_scene->GetEnviroment();
        env.skyboxCubemap = m_skyCubemaps["clear_night"];
        m_engine->GetRenderer()->SetSkyMode(eSkyMode::SkySphere);

        // camera
        {
            m_mainCamera = m_scene->GetCamera();
            auto transform = m_mainCamera->GetTransform();
            transform->SetLocalPosition(0.0f, 5.0f, -12.0f);
            transform->SetRotationByDegrees({ 15.0f, 0.0f, 0.0f });
        }
        SceneSerializer serializer(m_scene);
        serializer.Deserialize("Assets/Scenes/Lighting.scene");
        /*
        m_scene = m_engine->NewScene();
        m_scene->SetName("Lighting");

        auto& env = m_scene->GetEnviroment();
        env.skyboxCubemap = m_skyCubemaps["clear_night"];
        m_engine->GetRenderer()->SetSkyMode(eSkyMode::SkySphere);

        // camera
        {
            m_mainCamera = m_scene->GetCamera();
            auto transform = m_mainCamera->GetTransform();
            transform->SetLocalPosition(0.0f, 5.0f, -12.0f);
            transform->SetRotationByDegrees({ 15.0f, 0.0f, 0.0f });
        }

        // Lights
        {
            // Dir Light
            {
                auto renderer = m_engine->GetRenderer();
                renderer->SetLightDir(1.0f, -1.0f, -1.0f);
            }

            // Point Light Red
            {
                auto gameObject = createPointLight();
                gameObject->SetName("PointLight_Red");

                auto transform = gameObject->GetTransform();
                transform->SetLocalPosition(2.5f, 2.0f, -2.5f);

                auto light = gameObject->GetComponent<Light>();
                light->SetColor(1.0f, 0.0f, 0.0f);
                light->SetIntensity(8.0f);
                light->SetRange(5.0f);
            }

            // Point Light Green
            {
                auto gameObject = createPointLight();
                gameObject->SetName("PointLight_Green");

                auto transform = gameObject->GetTransform();
                transform->SetLocalPosition(-2.5f, 2.0f, 0.0f);

                auto light = gameObject->GetComponent<Light>();
                light->SetColor(0.0f, 1.0f, 0.0f);
                light->SetIntensity(8.0f);
                light->SetRange(5.0f);
            }

            // Point Light Blue
            {
                auto gameObject = createPointLight();
                gameObject->SetName("PointLight_Blue");

                auto transform = gameObject->GetTransform();
                transform->SetLocalPosition(-1.0f, 2.0f, 3.0f);

                auto light = gameObject->GetComponent<Light>();
                light->SetColor(0.0f, 0.0f, 1.0f);
                light->SetIntensity(8.0f);
                light->SetRange(5.0f);
            }

            // Point Light Yellow
            {
                auto gameObject = createPointLight();
                gameObject->SetName("PointLight_Yellow");

                auto transform = gameObject->GetTransform();
                transform->SetLocalPosition(2.5f, 2.0f, 0.0f);

                auto light = gameObject->GetComponent<Light>();
                light->SetColor(1.0f, 1.0f, 0.0f);
                light->SetIntensity(8.0f);
                light->SetRange(5.0f);;
            }

            // SpotLight White
            {
                auto gameObject = createSpotLight();
                gameObject->SetName("SpotLight_White");

                auto transform = gameObject->GetTransform();
                transform->SetLocalPosition(-2.5f, 2.5f, -2.5f);
                transform->SetLocalRotationByDegrees(0.0f, 0.0f, 0.0f);

                auto light = gameObject->GetComponent<Light>();
                light->SetInnerAngleDegrees(13.0f);
                light->SetOuterAngleDegrees(13.0f);
                light->SetIntensity(8.0f);
                light->SetRange(10.0f);
            }

            // SpotLight Purple
            {
                auto gameObject = createSpotLight();
                gameObject->SetName("SpotLight_Purple");

                auto transform = gameObject->GetTransform();
                transform->SetLocalPosition(2.5f, 2.5f, 3.0f);
                transform->SetLocalRotationByDegrees(90.0f, 0.0f, 0.0f);

                auto light = gameObject->GetComponent<Light>();
                light->SetIntensity(8.0f);
                light->SetRange(10.0f);
            }
        }

        // Objects
        {
            // bottom
            {
                auto bottom = m_scene->CreateGameObject();
                bottom->SetName("Bottom");
                auto meshRenderer = bottom->AddComponent<MeshRenderer>();
                meshRenderer->SetMesh(MeshManager::Get().GetMesh("Plane"));

                auto mtrl = MaterialManager::Get().GetMaterial("Assets/Materials/Metal_Plate");//"Marble_Cliff_06");
                meshRenderer->SetMaterial(mtrl);
            }

            // wall
            {
                auto wall = m_scene->CreateGameObject();
                wall->SetName("Bottom");
                auto meshRenderer = wall->AddComponent<MeshRenderer>();
                meshRenderer->SetMesh(MeshManager::Get().GetMesh("Quad"));

                auto mtrl = MaterialManager::Get().GetMaterial("Assets/Materials/Tiles106");
                meshRenderer->SetMaterial(mtrl);

                auto transform = wall->GetTransform();
                transform->SetLocalPosition(-2.5f, 2.5f, 5.0f);
                transform->SetScale(5.0f, 5.0f, 1.0f);
            }

            // Cube metal plate
            {
                auto cube = m_scene->CreateGameObject();
                auto meshRenderer = cube->AddComponent<MeshRenderer>();
                meshRenderer->SetMesh(MeshManager::Get().GetMesh("Cube"));
                cube->SetName("Cube");

                auto mtrl = MaterialManager::Get().GetMaterial("Assets/Materials/Marble_Cliff_06");// "Metal_Plate");
                meshRenderer->SetMaterial(mtrl);

                auto transform = cube->GetTransform();
                transform->SetLocalPosition(-2.0f, 0.5f, 3.0f);
            }

            // Cube Rusty Metal
            {
                auto cube = m_scene->CreateGameObject();
                auto meshRenderer = cube->AddComponent<MeshRenderer>();
                meshRenderer->SetMesh(MeshManager::Get().GetMesh("Cube"));
                cube->SetName("Quad_Left");

                auto mtrl = MaterialManager::Get().GetMaterial("Assets/Materials/Rusty_Metal_Grid");
                meshRenderer->SetMaterial(mtrl);

                cube->GetTransform()->SetLocalPosition(2.5f, 0.5f, 3.0f);
            }

            // Cube Stacked Brick Wall
            {
                auto cube = m_scene->CreateGameObject();
                auto meshRenderer = cube->AddComponent<MeshRenderer>();
                meshRenderer->SetMesh(MeshManager::Get().GetMesh("Cube"));
                cube->SetName("Quad_Right");

                auto mtrl = MaterialManager::Get().GetMaterial("Assets/Materials/Stacked_Brick_Wall");
                meshRenderer->SetMaterial(mtrl);

                auto transform = cube->GetTransform();
                transform->SetLocalPosition(2.0f, 0.5f, 0.0f);
            }

            // Spherer
            {
                auto sphere = m_scene->CreateGameObject();
                auto meshRenderer = sphere->AddComponent<MeshRenderer>();
                meshRenderer->SetMesh(MeshManager::Get().GetMesh("Sphere"));
                sphere->SetName("Sphere");

                auto mtrl = MaterialManager::Get().GetMaterial("Assets/Materials/Metal White");
                meshRenderer->SetMaterial(mtrl);

                auto transform = sphere->GetTransform();
                transform->SetLocalPosition(0.0f, 1.0f, -3.0f);
            }

            // Capsule
            {
                auto capsule = m_scene->CreateGameObject();
                auto meshRenderer = capsule->AddComponent<MeshRenderer>();
                meshRenderer->SetMesh(MeshManager::Get().GetMesh("Capsule"));
                capsule->SetName("Capsule");

                auto mtrl = MaterialManager::Get().GetMaterial("Assets/Materials/Rust_Metal_05");
                meshRenderer->SetMaterial(mtrl);

                auto transform = capsule->GetTransform();
                transform->SetLocalPosition(-3.5f, 1.0f, -3.0f);
            }

            // Triangle
            {
                auto triangle = m_scene->CreateGameObject();
                auto meshRenderer = triangle->AddComponent<MeshRenderer>();
                meshRenderer->SetMesh(MeshManager::Get().GetMesh("Triangle"));
                triangle->SetName("Triangle");

                auto mtrl = MaterialManager::Get().GetMaterial("Assets/Materials/Orange");
                meshRenderer->SetMaterial(mtrl);

                auto transform = triangle->GetTransform();
                transform->SetLocalPosition(-1.5f, 1.0f, -3.0f);

            }
        }
        */
    }

    void Sandbox::showMaterial()
    {
        if (!m_selectedMaterial)
            return;

        // name
        std::string mtrlName = m_selectedMaterial->GetName();
        ImGui::Text(mtrlName.c_str());

        // transparent
        ImGui::PushID("RenderingMode");
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 150.0f);
        ImGui::Text("Rendering Mode");
        ImGui::NextColumn();
        int curTransparent = m_selectedMaterial->IsTransparent() ? 1 : 0;
        ImGui::RadioButton("Opaque", &curTransparent, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Transparent", &curTransparent, 1);
        m_selectedMaterial->SetTransparent(curTransparent ? true : false);
        ImGui::Columns(1);
        ImGui::PopID();

        ImGui::Spacing();

        // 진하게...
        ImGui::Text("Maps");

        // albedo
        ImGui::PushID("Albedo");
        ImVec4 albedo = ImGuiHelper::ConvertImVec4(m_selectedMaterial->GetBaseColor());
        ImTextureID textureID = (ImTextureID)(m_selectedMaterial->GetMap(eMapType::Albedo) ?
            m_selectedMaterial->GetMap(eMapType::Albedo) : 0);
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 150.0f);
        if (ImGui::ImageButton("##Albedo", textureID, ImVec2(20, 20)))
        {
            const char* filters = "Texture Files (*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga)\0*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga\0"
                "All Files (*.*)\0*.*\0\0";
            auto openFile = FileUtils::OpenFile(filters, nullptr, "Assets/Textures");
            if (!openFile.empty())
            {
                //auto relPath = std::filesystem::relative(openFile, ResourceManager::GetResourcePath());
                //m_selectedMaterial->SetMap(eMapType::Diffuse, ResourceManager::Load<Texture2D>(relPath.replace_extension()));
            }
            else
            {
                m_selectedMaterial->SetMap(eMapType::Albedo, INVALID_TEXTURE_HANDLE);
            }
        }
        ImGui::SameLine();
        ImGui::Text("Albedo");
        ImGui::NextColumn();
        ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoInputs;
        ImGui::ColorEdit4("##AlbedoColor", (float*)&albedo, flags);
        m_selectedMaterial->SetBaseColor(ImGuiHelper::ConvertXMFloat4(albedo));
        ImGui::Columns(1);
        ImGui::PopID();

        // normal map
        ImGui::PushID("NormalMap");
        textureID = (ImTextureID)(m_selectedMaterial->GetMap(eMapType::Normal) ?
            m_selectedMaterial->GetMap(eMapType::Normal) : 0);
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 150.0f);
        if (ImGui::ImageButton("##Normal", textureID, ImVec2(20, 20)))
        {
            const char* filters = "Texture Files (*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga)\0*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga\0"
                "All Files (*.*)\0*.*\0\0";
            auto openFile = FileUtils::OpenFile(filters, nullptr, "Assets/Textures");
            if (!openFile.empty())
            {
                //auto relPath = std::filesystem::relative(openFile, ResourceManager::GetResourcePath());
                //m_selectedMaterial->SetMap(eMapType::Diffuse, ResourceManager::Load<Texture2D>(relPath.replace_extension()));
            }
            else
            {
                m_selectedMaterial->SetMap(eMapType::Normal, INVALID_TEXTURE_HANDLE);
            }
        }
        ImGui::SameLine();
        ImGui::Text("Normal");
        ImGui::NextColumn();
        ImGui::Columns(1);
        ImGui::PopID();

        textureID = (ImTextureID)(m_selectedMaterial->HasMap(eMapType::ORM) ?
            m_selectedMaterial->GetMap(eMapType::ORM) : 0);

        // Occlusion Map은 일단 제외

        // Roughness Map
        if (textureID == 0)
        {
            textureID = (ImTextureID)(m_selectedMaterial->HasMap(eMapType::Roughness) ?
                m_selectedMaterial->GetMap(eMapType::Roughness) : 0);
        }
        ImGui::PushID("Roughness");
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 150.0f);
        if (ImGui::ImageButton("##Roughness", textureID, ImVec2(20, 20)))
        {
            const char* filters = "Texture Files (*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga)\0*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga\0"
                "All Files (*.*)\0*.*\0\0";
            auto openFile = FileUtils::OpenFile(filters, nullptr, "Assets/Textures");
            if (!openFile.empty())
            {
                //auto relPath = std::filesystem::relative(openFile, ResourceManager::GetResourcePath());
                //m_selectedMaterial->SetMap(eMapType::Diffuse, ResourceManager::Load<Texture2D>(relPath.replace_extension()));
            }
            else
            {
                m_selectedMaterial->SetMap(eMapType::Roughness, INVALID_TEXTURE_HANDLE);
            }
        }
        ImGui::SameLine();
        ImGui::Text("Roughness");
        ImGui::NextColumn();
        float roughnessFactor = m_selectedMaterial->GetRoughnessFactor();
        ImGui::SliderFloat("##RoughnessFactor", &roughnessFactor, 0.0f, 1.0f, "%.2f");
        m_selectedMaterial->SetRoughnessFactor(roughnessFactor);
        ImGui::Columns(1);
        ImGui::PopID();

        // 조금 번잡하지만 일단 회피용이다.
        textureID = (ImTextureID)(m_selectedMaterial->HasMap(eMapType::ORM) ?
            m_selectedMaterial->GetMap(eMapType::ORM) : 0);

        // Metalic Map
        if (textureID == 0)
        {
            textureID = (ImTextureID)(m_selectedMaterial->HasMap(eMapType::Metallic) ?
                m_selectedMaterial->GetMap(eMapType::Metallic) : 0);
        }
        ImGui::PushID("Metallic");
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 150.0f);
        if (ImGui::ImageButton("##Metalic", textureID, ImVec2(20, 20)))
        {
            const char* filters = "Texture Files (*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga)\0*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga\0"
                "All Files (*.*)\0*.*\0\0";
            auto openFile = FileUtils::OpenFile(filters, nullptr, "Assets/Textures");
            if (!openFile.empty())
            {
                //auto relPath = std::filesystem::relative(openFile, ResourceManager::GetResourcePath());
                //m_selectedMaterial->SetMap(eMapType::Diffuse, ResourceManager::Load<Texture2D>(relPath.replace_extension()));
            }
            else
            {
                m_selectedMaterial->SetMap(eMapType::Metallic, INVALID_TEXTURE_HANDLE);
            }
        }
        ImGui::SameLine();
        ImGui::Text("Metallic");
        ImGui::NextColumn();
        float metallicFactor = m_selectedMaterial->GetMetallicFactor();
        ImGui::SliderFloat("##MetallicFactor", &metallicFactor, 0.0f, 1.0f, "%.2f");
        m_selectedMaterial->SetMetallicFactor(metallicFactor);
        ImGui::Columns(1);
        ImGui::PopID();

        // Displacement Map
        ImGui::PushID("Displacement");
        textureID = (ImTextureID)(m_selectedMaterial->GetMap(eMapType::Displacement) ?
            m_selectedMaterial->GetMap(eMapType::Displacement) : 0);
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 150.0f);
        if (ImGui::ImageButton("##Displacement", textureID, ImVec2(20, 20)))
        {
            const char* filters = "Texture Files (*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga)\0*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga\0"
                "All Files (*.*)\0*.*\0\0";
            auto openFile = FileUtils::OpenFile(filters, nullptr, "Assets/Textures");
            if (!openFile.empty())
            {
                //auto relPath = std::filesystem::relative(openFile, ResourceManager::GetResourcePath());
                //m_selectedMaterial->SetMap(eMapType::Diffuse, ResourceManager::Load<Texture2D>(relPath.replace_extension()));
            }
            else
            {
                m_selectedMaterial->SetMap(eMapType::Displacement, INVALID_TEXTURE_HANDLE);
            }
        }
        ImGui::SameLine();
        ImGui::Text("Displacement");
        ImGui::NextColumn();
        float heightScale = m_selectedMaterial->GetHeightScale();
        if (ImGui::DragFloat("##heightScale", &heightScale, 0.005f, 0.0f, 2.0f, "%.3f"))
        {
            m_selectedMaterial->SetHeightScale(heightScale);
        }
        ImGui::Columns(1);
        ImGui::PopID();

        // tiling
        auto tiling = m_selectedMaterial->GetTiling();
        ImGui::PushID("Tiling");
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 150.0f);
        ImGui::Text("Tiling");
        ImGui::SameLine();
        ImGui::NextColumn();
        ImGui::Text("X");
        ImGui::SameLine();
        ImGui::DragFloat("##tilingX", &tiling.x, 0.01f, 0.1f, 0.0f, "%.2f");
        ImGui::Text("Y");
        ImGui::SameLine();
        ImGui::DragFloat("##tilingY", &tiling.y, 0.01f, 0.0f, 0.0f, "%.2f");
        m_selectedMaterial->SetTiling(tiling);
        ImGui::Columns(1);
        ImGui::PopID();

        // offset
        auto offset = m_selectedMaterial->GetOffset();
        ImGui::PushID("Offset");
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 150.0f);
        ImGui::Text("Offset");
        ImGui::SameLine();
        ImGui::NextColumn();
        ImGui::Text("X");
        ImGui::SameLine();
        ImGui::DragFloat("##offsetX", &offset.x, 0.01f, 0.0f, 0.0f, "%.2f");
        ImGui::Text("Y");
        ImGui::SameLine();
        ImGui::DragFloat("##offsetY", &offset.y, 0.01f, 0.0f, 0.0f, "%.2f");
        m_selectedMaterial->SetOffset(offset);
        ImGui::Columns(1);
        ImGui::PopID();
    }

    void Sandbox::showNewMaterial()
    {
        static auto newMaterial = MaterialManager::Get().CreateMaterial("New Material");

        // name
        std::string mtrlName = newMaterial->GetName();
        ImGui::Text(mtrlName.c_str());

        // transparent
        ImGui::PushID("RenderingMode");
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 150.0f);
        ImGui::Text("Rendering Mode");
        ImGui::NextColumn();
        int curTransparent = newMaterial->IsTransparent() ? 1 : 0;
        ImGui::RadioButton("Opaque", &curTransparent, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Transparent", &curTransparent, 1);
        newMaterial->SetTransparent(curTransparent ? true : false);
        ImGui::Columns(1);
        ImGui::PopID();

        ImGui::Spacing();

        // 진하게...
        ImGui::Text("Maps");

        // albedo
        ImGui::PushID("Albedo");
        ImVec4 albedo = ImGuiHelper::ConvertImVec4(newMaterial->GetBaseColor());
        ImTextureID textureID = (ImTextureID)(newMaterial->GetMap(eMapType::Albedo) ?
            newMaterial->GetMap(eMapType::Albedo) : 0);
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 150.0f);
        if (ImGui::ImageButton("##Albedo", textureID, ImVec2(20, 20)))
        {
            const char* filters = "Texture Files (*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga)\0*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga\0"
                "All Files (*.*)\0*.*\0\0";
            auto openFile = FileUtils::OpenFile(filters, nullptr, "Assets/Textures");
            if (!openFile.empty())
            {
                //auto relPath = std::filesystem::relative(openFile, ResourceManager::GetResourcePath());
                //newMaterial->SetMap(eMapType::Diffuse, ResourceManager::Load<Texture2D>(relPath.replace_extension()));
            }
            else
            {
                newMaterial->SetMap(eMapType::Albedo, INVALID_TEXTURE_HANDLE);
            }
        }
        ImGui::SameLine();
        ImGui::Text("Albedo");
        ImGui::NextColumn();
        ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoInputs;
        ImGui::ColorEdit4("##AlbedoColor", (float*)&albedo, flags);
        newMaterial->SetBaseColor(ImGuiHelper::ConvertXMFloat4(albedo));
        ImGui::Columns(1);
        ImGui::PopID();

        // normal map
        ImGui::PushID("NormalMap");
        textureID = (ImTextureID)(newMaterial->GetMap(eMapType::Normal) ?
            newMaterial->GetMap(eMapType::Normal) : 0);
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 150.0f);
        if (ImGui::ImageButton("##Normal", textureID, ImVec2(20, 20)))
        {
            const char* filters = "Texture Files (*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga)\0*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga\0"
                "All Files (*.*)\0*.*\0\0";
            auto openFile = FileUtils::OpenFile(filters, nullptr, "Assets/Textures");
            if (!openFile.empty())
            {
                //auto relPath = std::filesystem::relative(openFile, ResourceManager::GetResourcePath());
                //newMaterial->SetMap(eMapType::Diffuse, ResourceManager::Load<Texture2D>(relPath.replace_extension()));
            }
            else
            {
                newMaterial->SetMap(eMapType::Normal, INVALID_TEXTURE_HANDLE);
            }
        }
        ImGui::SameLine();
        ImGui::Text("Normal");
        ImGui::NextColumn();
        ImGui::Columns(1);
        ImGui::PopID();

        textureID = (ImTextureID)(newMaterial->HasMap(eMapType::ORM) ?
            newMaterial->GetMap(eMapType::ORM) : 0);

        // Occlusion Map은 일단 제외

        // Roughness Map
        if (textureID == 0)
        {
            textureID = (ImTextureID)(newMaterial->HasMap(eMapType::Roughness) ?
                newMaterial->GetMap(eMapType::Roughness) : 0);
        }
        ImGui::PushID("Roughness");
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 150.0f);
        if (ImGui::ImageButton("##Roughness", textureID, ImVec2(20, 20)))
        {
            const char* filters = "Texture Files (*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga)\0*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga\0"
                "All Files (*.*)\0*.*\0\0";
            auto openFile = FileUtils::OpenFile(filters, nullptr, "Assets/Textures");
            if (!openFile.empty())
            {
                //auto relPath = std::filesystem::relative(openFile, ResourceManager::GetResourcePath());
                //newMaterial->SetMap(eMapType::Diffuse, ResourceManager::Load<Texture2D>(relPath.replace_extension()));
            }
            else
            {
                newMaterial->SetMap(eMapType::Roughness, INVALID_TEXTURE_HANDLE);
            }
        }
        ImGui::SameLine();
        ImGui::Text("Roughness");
        ImGui::NextColumn();
        float roughnessFactor = newMaterial->GetRoughnessFactor();
        ImGui::SliderFloat("##RoughnessFactor", &roughnessFactor, 0.0f, 1.0f, "%.2f");
        newMaterial->SetRoughnessFactor(roughnessFactor);
        ImGui::Columns(1);
        ImGui::PopID();

        // 조금 번잡하지만 일단 회피용이다.
        textureID = (ImTextureID)(newMaterial->HasMap(eMapType::ORM) ?
            newMaterial->GetMap(eMapType::ORM) : 0);

        // Metalic Map
        if (textureID == 0)
        {
            textureID = (ImTextureID)(newMaterial->HasMap(eMapType::Metallic) ?
                newMaterial->GetMap(eMapType::Metallic) : 0);
        }
        ImGui::PushID("Metallic");
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 150.0f);
        if (ImGui::ImageButton("##Metalic", textureID, ImVec2(20, 20)))
        {
            const char* filters = "Texture Files (*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga)\0*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga\0"
                "All Files (*.*)\0*.*\0\0";
            auto openFile = FileUtils::OpenFile(filters, nullptr, "Assets/Textures");
            if (!openFile.empty())
            {
                //auto relPath = std::filesystem::relative(openFile, ResourceManager::GetResourcePath());
                //newMaterial->SetMap(eMapType::Diffuse, ResourceManager::Load<Texture2D>(relPath.replace_extension()));
            }
            else
            {
                newMaterial->SetMap(eMapType::Metallic, INVALID_TEXTURE_HANDLE);
            }
        }
        ImGui::SameLine();
        ImGui::Text("Metallic");
        ImGui::NextColumn();
        float metallicFactor = newMaterial->GetMetallicFactor();
        ImGui::SliderFloat("##MetallicFactor", &metallicFactor, 0.0f, 1.0f, "%.2f");
        newMaterial->SetMetallicFactor(metallicFactor);
        ImGui::Columns(1);
        ImGui::PopID();

        // Displacement Map
        ImGui::PushID("Displacement");
        textureID = (ImTextureID)(newMaterial->GetMap(eMapType::Displacement) ?
            newMaterial->GetMap(eMapType::Displacement) : 0);
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 150.0f);
        if (ImGui::ImageButton("##Displacement", textureID, ImVec2(20, 20)))
        {
            const char* filters = "Texture Files (*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga)\0*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga\0"
                "All Files (*.*)\0*.*\0\0";
            auto openFile = FileUtils::OpenFile(filters, nullptr, "Assets/Textures");
            if (!openFile.empty())
            {
                //auto relPath = std::filesystem::relative(openFile, ResourceManager::GetResourcePath());
                //newMaterial->SetMap(eMapType::Diffuse, ResourceManager::Load<Texture2D>(relPath.replace_extension()));
            }
            else
            {
                newMaterial->SetMap(eMapType::Displacement, INVALID_TEXTURE_HANDLE);
            }
        }
        ImGui::SameLine();
        ImGui::Text("Displacement");
        ImGui::NextColumn();
        float heightScale = newMaterial->GetHeightScale();
        if (ImGui::DragFloat("##heightScale", &heightScale, 0.005f, 0.0f, 2.0f, "%.3f"))
        {
            newMaterial->SetHeightScale(heightScale);
        }
        ImGui::Columns(1);
        ImGui::PopID();
    }

    void Sandbox::popupMaterials()
    {
        if (ImGui::BeginPopup("Materials"))
        {
            ImGui::Text("Select Material");
            ImGui::Separator();

            // 1. MaterialManager에서 생성 순서가 보장된 벡터 목록 가져오기
            const auto& materialList = MaterialManager::Get().GetAllMaterials();

            if (materialList.empty())
            {
                ImGui::TextDisabled("No materials loaded.");
            }
            else
            {
                // 벡터 순회 (생성/로드된 순서대로 출력)
                for (Material* material : materialList)
                {
                    if (!material) continue;

                    // 머티리얼 이름을 가져오는 함수가 있다고 가정 (예: GetName())
                    std::string name = material->GetName();

                    if (ImGui::Selectable(name.c_str()))
                    {
                        if (m_selectedObject && m_selectedObject->HasComponent<MeshRenderer>())
                        {
                            auto meshRenderer = m_selectedObject->GetComponent<MeshRenderer>();
                            meshRenderer->SetMaterial(material);
                        }
                        ImGui::CloseCurrentPopup();
                    }
                }
            }

            // 2. 구분선 및 파일 불러오기 옵션
            ImGui::Separator();

            if (ImGui::Selectable("Load File..."))
            {
                const char* filters = "Texture Files (*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga)\0*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga\0"
                    "All Files (*.*)\0*.*\0\0";
                auto openFile = FileUtils::OpenFile(filters, nullptr, "Assets/Textures");
                if (!openFile.empty())
                {
                    //auto relPath = std::filesystem::relative(openFile, ResourceManager::GetResourcePath());
                    //newMaterial->SetMap(eMapType::Diffuse, ResourceManager::Load<Texture2D>(relPath.replace_extension()));
                }

                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    GameObject* Sandbox::createPointLight()
    {
        auto gameObject = m_scene->CreateGameObject();
        gameObject->SetName("PointLight");

        auto light = gameObject->AddComponent<Light>();
        light->SetLightType(eLightType::Point);

        auto transform = gameObject->GetTransform();
        transform->SetLocalPosition(0.0f, 5.0f, 0.0f);

        return gameObject;
    }

    GameObject* Sandbox::createSpotLight()
    {
        auto gameObject = m_scene->CreateGameObject();
        gameObject->SetName("SpotLight");

        auto light = gameObject->AddComponent<Light>();
        light->SetLightType(eLightType::Spot);

        auto transform = gameObject->GetTransform();
        transform->SetLocalPosition(0.0f, 5.0f, 0.0f);
        transform->SetRotationByDegrees({ 90.0f, 0.0f, 0.0f });

        return gameObject;
    }
}