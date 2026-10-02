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
#include "Rendering/Material.h"
#include "Graphics/RenderTexture.h"

namespace Dive
{
    namespace
    {
        constexpr float BOOST_SPEED = 10.0f;
        constexpr float MIN_SPEED = 0.5f;
        constexpr float MAX_SPEED = 99.0f;
    }

    static void DrawFloatControl(const std::string& label, float& value, float resetValue = 0.0f, float columnWidth = 100.0f)
    {
        ImGui::PushID(label.c_str());

        ImGui::Columns(2);

        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text(label.c_str());
        ImGui::NextColumn();

        ImGui::DragFloat("##X", &value, 0.1f, 0.0f, 0.0f, "%.2f");

        ImGui::Columns(1);

        ImGui::PopID();
    }

    static void DrawVec2Control(const std::string& label, DirectX::XMFLOAT2& values, float resetValue = 0.0f, float columnWidth = 100.0f, const std::string& a = "X", const std::string& b = "Y")
    {
        ImGuiIO& io = ImGui::GetIO();
        auto pBoldFont = io.Fonts->Fonts[1];

        ImGui::PushID(label.c_str());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text(label.c_str());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

        //float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        float lineHeight = ImGui::GetFontSize() + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushFont(pBoldFont);
        if (ImGui::Button(a.c_str(), buttonSize))
            values.x = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushFont(pBoldFont);
        if (ImGui::Button(b.c_str(), buttonSize))
            values.y = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();

        ImGui::Columns(1);

        ImGui::PopID();
    }

    static void DrawVec3Control(const std::string& label, DirectX::XMFLOAT3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
    {
        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];

        ImGui::PushID(label.c_str());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text(label.c_str());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

        float lineHeight = ImGui::GetFontSize() + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("X", buttonSize))
            values.x = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Y", buttonSize))
            values.y = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Z", buttonSize))
            values.z = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();

        ImGui::Columns(1);

        ImGui::PopID();
    }

    static DirectX::XMFLOAT4 ImVec4ToXMFloat4(const ImVec4& value)
    {
        return { value.x, value.y, value.z, value.w };
    }

    static ImVec4 XMFloat4ToImVec4(const DirectX::XMFLOAT4& value)
    {
        return { value.x, value.y, value.z, value.w };
    }

    Sandbox::Sandbox(const SandboxInit& init)
    {
        m_engine = std::make_unique<Engine>(init.engin_init);
        m_gui = std::make_unique<ImGuiManager>(m_engine->GetGraphics());

        loadResources();
 
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
                    if (ImGui::IsKeyPressed(ImGuiKey_Escape))
                    {
                        if (m_showEnviromentMenu) m_showEnviromentMenu = false;
                        else if (m_showInspectorMenu) m_showInspectorMenu = false;
                        else if (m_selectedObject) setSelectedObject(nullptr);
                        else m_showQuitMenu = !m_showQuitMenu;
                    }

                    if (ImGui::IsKeyPressed(ImGuiKey_F1))
                    {
                        m_showEnviromentMenu = !m_showEnviromentMenu;
                    }

                    if (ImGui::IsKeyPressed(ImGuiKey_I) && m_selectedObject)
                    {
                        m_showInspectorMenu = !m_showInspectorMenu;
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

            
            if (m_mainCamera != nullptr && !m_scene->GetLightQueue().empty())
            {
                auto cameraCom = m_mainCamera->GetComponent<Camera>();
                DirectX::XMMATRIX view = cameraCom->GetViewMatrix();
                DirectX::XMMATRIX proj = cameraCom->GetProjectionMatrix();
                DirectX::XMMATRIX viewProj = DirectX::XMMatrixMultiply(view, proj);

                ImDrawList* drawList = ImGui::GetWindowDrawList();

                for (const auto& lightObject : m_scene->GetLightQueue())
                {
                    auto type = lightObject->GetComponent<Light>()->GetLightType();
                    if (type == eLightType::Directional)
                        continue;

                    DirectX::XMVECTOR worldPos = lightObject->GetTransform()->GetPositionVector();

                    // 1. 3D 월드 좌표를 클립/NDC 공간으로 변환
                    DirectX::XMVECTOR projected = DirectX::XMVector3TransformCoord(worldPos, viewProj);

                    float ndcX = DirectX::XMVectorGetX(projected);
                    float ndcY = DirectX::XMVectorGetY(projected);
                    float ndcZ = DirectX::XMVectorGetZ(projected);

                    // 2. 카메라 뒤쪽에 있는 라이트는 무시 (클리핑)
                    if (ndcZ < 0.0f || ndcZ > 1.0f) continue;

                    // 3. NDC(-1~1)를 뷰포트 내의 실제 픽셀 화면 좌표로 변환
                    float screenX = viewportPos.x + (1.0f + ndcX) * 0.5f * viewportSize.x;
                    float screenY = viewportPos.y + (1.0f - ndcY) * 0.5f * viewportSize.y; // DirectX는 Y축 아래가 정방향

                    // 4. ImGui로 아이콘 그리기 (예: 32x32 크기)
                    float iconSize = 64.0f;
                    auto srv = (type == eLightType::Point) ? 
                        TextureManager::Get().GetTextureView(m_pointLightIcon) : 
                        TextureManager::Get().GetTextureView(m_spotLightIcon);

                    ImTextureID lightIconID = (ImTextureID)srv;

                    drawList->AddImage(lightIconID,
                        ImVec2(screenX - iconSize * 0.5f, screenY - iconSize * 0.5f),
                        ImVec2(screenX + iconSize * 0.5f, screenY + iconSize * 0.5f));
                }
            }
            // ==========================================

            if (m_isSceneViewHovered && !ImGuizmo::IsUsing() && !ImGuizmo::IsOver() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                bool clickedLightIcon = false;

                // 1. 먼저 라이트 아이콘들을 클릭했는지 순회하며 검사
                if (m_mainCamera != nullptr && !m_scene->GetLightQueue().empty())
                {
                    auto cameraCom = m_mainCamera->GetComponent<Camera>();
                    DirectX::XMMATRIX view = cameraCom->GetViewMatrix();
                    DirectX::XMMATRIX proj = cameraCom->GetProjectionMatrix();
                    DirectX::XMMATRIX viewProj = DirectX::XMMatrixMultiply(view, proj);

                    ImVec2 mousePos = ImGui::GetMousePos(); // 현재 마우스의 화면 픽셀 좌표
                    float iconSize = 64.0f;
                    float halfSize = iconSize * 0.5f;

                    for (const auto& lightObject : m_scene->GetLightQueue())
                    {
                        auto type = lightObject->GetComponent<Light>()->GetLightType();
                        if (type == eLightType::Directional) continue; // 디렉셔널은 아이콘 위치가 모호하므로 패스

                        DirectX::XMVECTOR worldPos = lightObject->GetTransform()->GetPositionVector();
                        DirectX::XMVECTOR projected = DirectX::XMVector3TransformCoord(worldPos, viewProj);

                        float ndcX = DirectX::XMVectorGetX(projected);
                        float ndcY = DirectX::XMVectorGetY(projected);
                        float ndcZ = DirectX::XMVectorGetZ(projected);

                        if (ndcZ < 0.0f || ndcZ > 1.0f) continue;

                        float screenX = viewportPos.x + (1.0f + ndcX) * 0.5f * viewportSize.x;
                        float screenY = viewportPos.y + (1.0f - ndcY) * 0.5f * viewportSize.y;

                        // 아이콘의 2D 사각형 영역(Bounding Box) 계산
                        ImVec2 minBound(screenX - halfSize, screenY - halfSize);
                        ImVec2 maxBound(screenX + halfSize, screenY + halfSize);

                        // 마우스 커서가 아이콘 사각형 내부에 있는지 확인
                        if (mousePos.x >= minBound.x && mousePos.x <= maxBound.x &&
                            mousePos.y >= minBound.y && mousePos.y <= maxBound.y)
                        {
                            // 현재 ObjectI가 MeshRenderer에 존재하지만
                            // Light Object는 MeshRenderer를 가지고 있지 않다.
                            setSelectedObject(lightObject); // 라이트 오브젝트 선택!
                            clickedLightIcon = true;
                            break;
                        }
                    }
                }

                // 2. 라이트 아이콘을 클릭하지 않았을 때만 기존 3D 셰이프(메쉬) 픽킹 수행
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
            /*
            if (m_isSceneViewHovered && !ImGuizmo::IsUsing() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
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
            */
            drawCanvasContextMenu();

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
        showInspector();
        showQuit();
    }

    void Sandbox::drawCanvasContextMenu()
    {
        if (m_isSceneViewHovered && !ImGuizmo::IsUsing() && !ImGuizmo::IsOver() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            if (m_selectedObject)
            {
                auto renderer = m_engine->GetRenderer();
                renderer->ProcessPicking();
                auto data = renderer->GetPickingData();

                m_contextTargetObject = m_scene->GetGameObjectByObjectID(data.id);
            }
            ImGui::OpenPopup("SceneViewContextMenu");
        }

        if (ImGui::BeginPopup("SceneViewContextMenu"))
        {
            if(m_selectedObject && m_contextTargetObject != nullptr)
            {
                if (m_selectedObject != m_contextTargetObject)
                {
                    setSelectedObject(m_contextTargetObject);
                }

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
                
                ImGui::Separator();

                ImGui::MenuItem("Inspector", "I", &m_showInspectorMenu);
            }
            else
            {
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
                    if (ImGui::MenuItem("Import"))
                    {
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Light"))
                {
                    if (ImGui::MenuItem("Point Light"))
                    {
                        auto gameObject = m_scene->CreateGameObject();
                        gameObject->SetName("PointLight");
                        
                        auto light = gameObject->AddComponent<Light>();
                        light->SetLightType(eLightType::Point);
                        light->SetColor(1.0f, 1.0f, 1.0f);
                        
                        auto transform = gameObject->GetTransform();
                        transform->SetPosition(0.0f, 5.0f, 0.0f);
                        
                        // 임시다. ObjectID를 위해 추가했다.
                        auto meshRenderer = gameObject->AddComponent<MeshRenderer>();
                    }
                    if (ImGui::MenuItem("Spot Light"))
                    {
                        auto gameObject = m_scene->CreateGameObject();
                        gameObject->SetName("SpotLight");

                        auto light = gameObject->AddComponent<Light>();
                        light->SetLightType(eLightType::Spot);
                        light->SetColor(1.0f, 1.0f, 1.0f);

                        auto transform = gameObject->GetTransform();
                        transform->SetPosition(0.0f, 5.0f, 0.0f);
                        transform->SetRotationByDegrees({ 90.0f, 0.0f, 0.0f });

                        // 임시다. ObjectID를 위해 추가했다.
                        auto meshRenderer = gameObject->AddComponent<MeshRenderer>();
                    }
                    ImGui::EndMenu();
                }
            }

            ImGui::EndPopup();
        }
    }

    void Sandbox::showEnviroment()
    {
        if (!m_showEnviromentMenu)
            return;

        if (m_showInspectorMenu) m_showInspectorMenu = false;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImVec2 windowSize = { 350.0f, 400.0f };
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
            ImGuiWindowFlags_HorizontalScrollbar;

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));

        if (ImGui::Begin("Environment", &m_showEnviromentMenu, flags))
        {
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::CollapsingHeader("Sky"))
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
                    ImGui::Combo("cube map", &item_current, items, IM_COUNTOF(items));

                    auto handle = m_skyCubemaps[items[item_current]];
                    m_scene->GetEnviroment().skyboxCubemap = handle;    // 이 부분이 마음에 들지 않는다.
                }
                if (currentSkyMode == 1)
                {
                    const char* items[] = { "night_puresky", "dawn", "clear_night"};
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

            // 현재 dir light가 Renderer에 귀속되어 있다.
            // weather 구현 중 같이 묶인 것 같다.
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::CollapsingHeader("Lighting"))
            {
                auto renderer = m_engine->GetRenderer();

                // direction
                // -1.0f ~ 1.0 사이로 제한해야 한다.
                // 그림판3D처럼 제어하고 싶다.
                {
                    DirectX::XMFLOAT3 dir = renderer->GetLightDir();
                    DrawVec3Control("Direction", dir, 0.0f, 100.0f);
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

            ImGui::Separator();
            
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::CollapsingHeader("Effect"))
            {
                // smog
                // rain
            }
        }
        ImGui::End();
        ImGui::PopStyleColor();
    }

    void Sandbox::showInspector()
    {
        if (!m_showInspectorMenu || !m_selectedObject)
            return;

        // 호출 순서때문에 이게 안먹힌다.
        //if (m_showEnviromentMenu) m_showEnviromentMenu = false;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos({ viewport->WorkSize.x - 370.0f, viewport->WorkPos.y });
        ImGui::SetNextWindowSize({ 370.0f, viewport->WorkSize.y });

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_HorizontalScrollbar;

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));

        if (ImGui::Begin("Inspector", &m_showInspectorMenu, flags))
        {
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

            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::CollapsingHeader("Transform"))
            {
                auto transform = m_selectedObject->GetTransform();

                auto position = transform->GetLocalPosition();
                DrawVec3Control("Position", position, 0.0f, 100.0f);
                transform->SetLocalPosition(position);

                auto rotation = transform->GetLocalRotationDegrees();
                DrawVec3Control("Rotation", rotation, 0.0f, 100.0f);
                transform->SetLocalRotationByDegrees(rotation);

                auto scale = transform->GetLocalScale();
                DrawVec3Control("Scale", scale, 1.0f, 100.0f);
                transform->SetLocalScale(scale);
            }

            ImGui::Separator();

            if (m_selectedObject->HasComponent<Light>())
            {
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                if (ImGui::CollapsingHeader("LIGHT"))
                {
                    auto light = m_selectedObject->GetComponent<Light>();

                    // type
                    ImGui::PushID("LightType");
                    ImGui::Columns(2);
                    ImGui::SetColumnWidth(0, 150.0f);
                    ImGui::Text("Type");
                    ImGui::NextColumn();
                    std::vector<const char*> lightTypes;
                    lightTypes.push_back("Direcitonal");
                    lightTypes.push_back("Point");
                    lightTypes.push_back("Spot");
                    int currentType = static_cast<int>(light->GetLightType());
                    ImGui::Combo("##LightType", &currentType, lightTypes.data(), static_cast<int>(lightTypes.size()));
                    light->SetLightType(static_cast<eLightType>(currentType));
                    ImGui::Columns(1);
                    ImGui::PopID();

                    if (currentType != static_cast<int>(eLightType::Directional))
                    {
                        // range
                        ImGui::PushID("LightRange");
                        ImGui::Columns(2);
                        ImGui::SetColumnWidth(0, 150.0f);
                        ImGui::Text("Range");
                        ImGui::NextColumn();
                        float range = light->GetRange();
                        ImGui::DragFloat("##LightRange", &range, 0.1f, 0.0f, 0.0f, "%.2f");
                        light->SetRange(range);
                        ImGui::Columns(1);
                        ImGui::PopID();

                        if (currentType == static_cast<int>(eLightType::Spot))
                        {
                            // spot angles
                            static DirectX::XMFLOAT2 spotAngles = { 0.0f, 0.0f };
                            spotAngles.x = 0;// light->GetInnerAngleDegrees();
                            spotAngles.y = 0;// light->GetOuterAngleDegrees();
                            //DrawVec2Control("Spot Angles", spotAngles, 0.0f, 150.0f, "I", "O");
                            //light->SetInnerAngleDegrees(spotAngles.x);
                            //light->SetOuterAngleDegrees(spotAngles.y);
                        }
                    }

                    // color
                    ImGui::PushID("LightColor");
                    ImVec4 color = { light->GetColor().x, light->GetColor().y, light->GetColor().z, 1.0f };
                    ImGui::Columns(2);
                    ImGui::SetColumnWidth(0, 150.0f);
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
                    ImGui::SetColumnWidth(0, 150.0f);
                    ImGui::Text("Intensity");
                    ImGui::NextColumn();
                    float intensity = light->GetIntensity();
                    ImGui::DragFloat("##LightIntensity", &intensity, 0.1f, 0.0f, 8.0f, "%.2f");
                    light->SetIntensity(intensity);
                    ImGui::Columns(1);
                    ImGui::PopID();
                }
            }


            ImGui::Separator();

            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::CollapsingHeader("Material"))
            {
                auto meshRenderer = m_selectedObject->GetComponent<MeshRenderer>();
                auto material = meshRenderer->GetMaterial();

                // 여기에서 이미 생성해 놓은 것을 선택토록 하자.
                // name
                ImGui::PushID("MaterialName");
                std::string mtrlName = material->GetName();
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, 150.0f);
                ImGui::Text("Material");
                ImGui::NextColumn();
                ImGui::InputText("##MtrlName", &mtrlName);
                material->SetName(mtrlName);
                ImGui::SameLine();
                if (ImGui::Button("...##Material"))
                {
                    /*
                    const char* pFilter = "Material Files (*.mat)\0*.mat\0All Files (*.*)\0*.*\0\0";
                    auto openFile = FileUtils::OpenFile(pFilter, nullptr, "Assets/Resources/Materials");
                    if (!openFile.empty())
                    {
                        auto relPath = std::filesystem::relative(openFile, ResourceManager::GetResourcePath());
                        staticMeshRenderer->SetMaterial(ResourceManager::Load<Material>(relPath.replace_extension()));
                    }
                    */
                }
                ImGui::Columns(1);
                ImGui::PopID();

                // transparent
                ImGui::PushID("RenderMode");
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, 150.0f);
                ImGui::Text("Render Mode");
                ImGui::NextColumn();
                int curTransparent = material->IsTransparent() ? 1 : 0;
                ImGui::RadioButton("Opaque", &curTransparent, 0);
                ImGui::SameLine();
                ImGui::RadioButton("Transparent", &curTransparent, 1);
                material->SetTransparent(curTransparent ? true : false);
                ImGui::Columns(1);
                ImGui::PopID();
                
                // albedo
                ImGui::PushID("Albedo");
                ImVec4 albedo = XMFloat4ToImVec4(material->GetBaseColor());
                ImTextureID textureID = (ImTextureID)(material->GetMap(eMapType::Albedo) ?
                    material->GetMap(eMapType::Albedo) : 0);
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, 150.0f);
                if (ImGui::ImageButton("##Albedo", textureID, ImVec2(20, 20)))
                {
                    /*
                    const char* pFilter = "Texture Files (*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga)\0*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga\0"
                        "All Files (*.*)\0*.*\0\0";
                    auto openFile = FileUtils::OpenFile(pFilter, nullptr, "Assets/Resources/Textures");
                    if (!openFile.empty())
                    {
                        auto relPath = std::filesystem::relative(openFile, ResourceManager::GetResourcePath());
                        material->SetMap(eMapType::Diffuse, ResourceManager::Load<Texture2D>(relPath.replace_extension()));
                    }
                    else
                    {
                        material->SetMap(eMapType::Diffuse, std::shared_ptr<Texture2D>(nullptr));
                    }
                    */
                }
                ImGui::SameLine();
                ImGui::Text("Albedo");
                ImGui::NextColumn();
                ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoInputs;
                ImGui::ColorEdit4("##AlbedoColor", (float*)&albedo, flags);
                material->SetBaseColor(ImVec4ToXMFloat4(albedo));
                ImGui::Columns(1);
                ImGui::PopID();

                // normal map
                ImGui::PushID("NormalMap");
                textureID = (ImTextureID)(material->GetMap(eMapType::Normal) ?
                    material->GetMap(eMapType::Normal) : 0);
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, 150.0f);
                if (ImGui::ImageButton("##Normal", textureID, ImVec2(20, 20)))
                {
                    /*
                    const char* pFilter = "Texture Files (*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga)\0*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga\0"
                        "All Files (*.*)\0*.*\0\0";
                    auto openFile = FileUtils::OpenFile(pFilter, nullptr, "Assets/Resources/Textures");
                    if (!openFile.empty())
                    {
                        auto relPath = std::filesystem::relative(openFile, ResourceManager::GetResourcePath());
                        material->SetMap(eMapType::Normal, ResourceManager::Load<Texture2D>(relPath.replace_extension()));
                    }
                    else
                    {
                        material->SetMap(eMapType::Normal, std::shared_ptr<Texture2D>(nullptr));
                    }
                    */
                }
                ImGui::SameLine();
                ImGui::Text("Normal Map");
                ImGui::NextColumn();
                ImGui::Columns(1);
                ImGui::PopID();

                textureID = (ImTextureID)(material->HasMap(eMapType::ORM) ?
                    material->GetMap(eMapType::ORM) : 0);

                // Occlusion Map은 일단 제외
              
                // Roughness Map
                if (textureID == 0)
                {
                    textureID = (ImTextureID)(material->HasMap(eMapType::Roughness) ?
                        material->GetMap(eMapType::Roughness) : 0);
                }
                ImGui::PushID("RoughnessMap");
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, 150.0f);
                if (ImGui::ImageButton("##Roughness", textureID, ImVec2(20, 20)))
                {
                }
                ImGui::SameLine();
                ImGui::Text("Roughness Map");
                ImGui::NextColumn();
                float roughnessFactor = material->GetRoughnessFactor();
                ImGui::SliderFloat("##RoughnessFactor", &roughnessFactor, 0.0f, 1.0f, "%.2f");
                material->SetRoughnessFactor(roughnessFactor);
                ImGui::Columns(1);
                ImGui::PopID();

                // 조금 번잡하지만 일단 회피용이다.
                textureID = (ImTextureID)(material->HasMap(eMapType::ORM) ?
                    material->GetMap(eMapType::ORM) : 0);

                // Metalic Map
                if (textureID == 0)
                {
                    textureID = (ImTextureID)(material->HasMap(eMapType::Metallic) ?
                        material->GetMap(eMapType::Metallic) : 0);
                }
                ImGui::PushID("MetallicMap");
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, 150.0f);
                if (ImGui::ImageButton("##Metalic", textureID, ImVec2(20, 20)))
                {
                }
                ImGui::SameLine();
                ImGui::Text("Metallic Map");
                ImGui::NextColumn();
                float metallicFactor = material->GetMetallicFactor();
                ImGui::SliderFloat("##MetallicFactor", &metallicFactor, 0.0f, 1.0f, "%.2f");
                material->SetMetallicFactor(metallicFactor);
                ImGui::Columns(1);
                ImGui::PopID();

                // Displacement Map
                ImGui::PushID("DisplacementMap");
                textureID = (ImTextureID)(material->GetMap(eMapType::Displacement) ?
                    material->GetMap(eMapType::Displacement) : 0);
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, 150.0f);
                if (ImGui::ImageButton("##Displacement", textureID, ImVec2(20, 20)))
                {
                }
                ImGui::SameLine();
                ImGui::Text("Displacement Map");
                ImGui::NextColumn();
                float heightScale = material->GetHeightScale();
                if (ImGui::DragFloat("##heightScale", &heightScale, 0.005f, 0.0f, 2.0f, "%.3f"))
                {
                    material->SetHeightScale(heightScale);
                }
                ImGui::Columns(1);
                ImGui::PopID();
            }
        }
        ImGui::End();
        ImGui::PopStyleColor();
    }

    void Sandbox::showQuit()
    {
        if (!m_showQuitMenu)
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

        if (ImGui::Begin("Quit", &m_showQuitMenu, flags))
        {
            // 버튼 크기 1.5배 적용 (135x26 -> 202.5x39)
            float buttonWidth = 202.5f;
            float buttonHeight = 39.0f;
            float windowWidth = ImGui::GetWindowSize().x;

            ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
            if (ImGui::Button("New", ImVec2(buttonWidth, buttonHeight)))
            {
            }

            // 2. Save
            ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
            if (ImGui::Button("Save", ImVec2(buttonWidth, buttonHeight)))
            {
                // TODO: 저장 로직
            }

            // 3. Load
            ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
            if (ImGui::Button("Load", ImVec2(buttonWidth, buttonHeight)))
            {
                // TODO: 로드 로직
            }

            // 4. Option
            ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
            if (ImGui::Button("Options", ImVec2(buttonWidth, buttonHeight)))
            {
                m_showQuitMenu = false;
                m_showEnviromentMenu = true;
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

    void Sandbox::newScene()
    {
        m_scene = m_engine->NewScene();
        m_scene->SetName("Sandbox");

        auto& env = m_scene->GetEnviroment();
        env.skyboxCubemap = m_skyCubemaps["clear_night"];
        m_engine->GetRenderer()->SetSkyMode(eSkyMode::SkySphere);

        // camera
        {
            m_mainCamera = m_scene->GetCamera();
            auto transform = m_mainCamera->GetTransform();
            transform->SetPosition(0.0f, 5.0f, -12.0f);
            transform->SetRotationByDegrees({15.0f, 0.0f, 0.0f});
        }

        // Lights
        {
            // Dir Light
            {
                auto renderer = m_engine->GetRenderer();
                renderer->SetLightDir(1.0f, -1.0f, 1.0f);

            }
            // Point Light Red
            {
                auto gameObject = m_scene->CreateGameObject();
                gameObject->SetName("PointLight_Red");

                auto light = gameObject->AddComponent<Light>();
                light->SetLightType(eLightType::Point);
                light->SetColor(1.0f, 0.0f, 0.0f);
                light->SetIntensity(8.0f);
                light->SetRange(5.0f);

                auto transform = gameObject->GetTransform();
                transform->SetPosition(2.5f, 2.0f, -2.5f);
                
                // 임시다.
                // MeshRenderer가 없어야 하지만
                // ObjectID때문에 일단 되살렸다.
                auto meshRenderer = gameObject->AddComponent<MeshRenderer>();
                //meshRenderer->SetMesh(MeshManager::Get().GetMesh("Quad"));

                //auto mtrl = MaterialManager::Get().CreateMaterial("PointLightIcon");
                //mtrl->SetMap(eMapType::Albedo, m_pointLightIcon);
                //mtrl->SetTransparent(true);
                //meshRenderer->SetMaterial(mtrl);
            }

            // Point Light Green
            {
                auto gameObject = m_scene->CreateGameObject();
                gameObject->SetName("PointLight_Green");

                auto light = gameObject->AddComponent<Light>();
                light->SetLightType(eLightType::Point);
                light->SetColor(0.0f, 1.0f, 0.0f);
                light->SetIntensity(8.0f);
                light->SetRange(5.0f);

                auto transform = gameObject->GetTransform();
                transform->SetPosition(-2.5f, 2.0f, 0.0f);

                auto meshRenderer = gameObject->AddComponent<MeshRenderer>();
                //meshRenderer->SetMesh(MeshManager::Get().GetMesh("Quad"));

                //auto mtrl = MaterialManager::Get().GetMaterial("PointLightIcon");
                //meshRenderer->SetMaterial(mtrl);
            }

            // Point Light Blue
            {
                auto gameObject = m_scene->CreateGameObject();
                gameObject->SetName("PointLight_Blue");

                auto light = gameObject->AddComponent<Light>();
                light->SetLightType(eLightType::Point);
                light->SetColor(0.0f, 0.0f, 1.0f);
                light->SetIntensity(8.0f);
                light->SetRange(5.0f);

                auto transform = gameObject->GetTransform();
                transform->SetPosition(-1.0f, 2.0f, 3.0f);
                
                auto meshRenderer = gameObject->AddComponent<MeshRenderer>();
                //meshRenderer->SetMesh(MeshManager::Get().GetMesh("Quad"));

                //auto mtrl = MaterialManager::Get().GetMaterial("PointLightIcon");
                //meshRenderer->SetMaterial(mtrl);
            }

            // Point Light Yellow
            {
                auto gameObject = m_scene->CreateGameObject();
                gameObject->SetName("PointLight");

                auto light = gameObject->AddComponent<Light>();
                light->SetLightType(eLightType::Point);
                light->SetColor(1.0f, 1.0f, 0.0f);
                light->SetIntensity(8.0f);
                light->SetRange(5.0f);

                auto transform = gameObject->GetTransform();
                transform->SetPosition(2.5f, 2.0f, 0.0f);
                
                auto meshRenderer = gameObject->AddComponent<MeshRenderer>();
                //meshRenderer->SetMesh(MeshManager::Get().GetMesh("Quad"));

                //auto mtrl = MaterialManager::Get().GetMaterial("PointLightIcon");
                //meshRenderer->SetMaterial(mtrl);
            }

            // SpotLight White
            {
                auto gameObject = m_scene->CreateGameObject();
                gameObject->SetName("SpotLight");

                auto light = gameObject->AddComponent<Light>();
                light->SetLightType(eLightType::Spot);
                light->SetColor(1.0f, 1.0f, 1.0f);
                light->SetIntensity(8.0f);
                light->SetRange(10.0f);

                auto transform = gameObject->GetTransform();
                transform->SetPosition(0.0f, 2.5f, -2.5f);
                transform->SetRotationByDegrees({ 90.0f, 0.0f, 0.0f });

                // 임시다. ObjectID를 위해 추가했다.
                auto meshRenderer = gameObject->AddComponent<MeshRenderer>();
            }

            // SpotLight Purple
            {
                auto gameObject = m_scene->CreateGameObject();
                gameObject->SetName("SpotLight");

                auto light = gameObject->AddComponent<Light>();
                light->SetLightType(eLightType::Spot);
                light->SetColor(1.0f, 0.0f, 1.0f);
                light->SetIntensity(8.0f);
                light->SetRange(10.0f);

                auto transform = gameObject->GetTransform();
                transform->SetPosition(2.5f, 2.5f, 3.0f);
                transform->SetRotationByDegrees({ 90.0f, 0.0f, 0.0f });

                // 임시다. ObjectID를 위해 추가했다.
                auto meshRenderer = gameObject->AddComponent<MeshRenderer>();
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

                auto mtrl = MaterialManager::Get().GetMaterial("Tiles106");
                meshRenderer->SetMaterial(mtrl);
            }

            // wall
            {
                auto wall = m_scene->CreateGameObject();
                wall->SetName("Bottom");
                auto meshRenderer = wall->AddComponent<MeshRenderer>();
                meshRenderer->SetMesh(MeshManager::Get().GetMesh("Quad"));

                auto mtrl = MaterialManager::Get().GetMaterial("Tiles106");
                meshRenderer->SetMaterial(mtrl);

                auto transform = wall->GetTransform();
                transform->SetPosition(-2.5f, 2.5f, 5.0f);
                transform->SetScale({ 5.0f, 5.0f, 1.0f });
            }

            // Cube metal plate
            {
                auto cube = m_scene->CreateGameObject();
                auto meshRenderer = cube->AddComponent<MeshRenderer>();
                meshRenderer->SetMesh(MeshManager::Get().GetMesh("Cube"));
                cube->SetName("Cube");

                auto mtrl = MaterialManager::Get().GetMaterial("Metal_Plate");
                meshRenderer->SetMaterial(mtrl);

                auto transform = cube->GetTransform();
                transform->SetPosition(-2.0f, 0.5f, 3.0f);
            }

            // Cube Rusty Metal
            {
                auto cube = m_scene->CreateGameObject();
                auto meshRenderer = cube->AddComponent<MeshRenderer>();
                meshRenderer->SetMesh(MeshManager::Get().GetMesh("Cube"));
                cube->SetName("Quad_Left");

                auto mtrl = MaterialManager::Get().GetMaterial("Rusty_Metal_Grid");
                meshRenderer->SetMaterial(mtrl);

                cube->GetTransform()->SetPosition(2.5f, 0.5f, 3.0f);
            }

            // Cube Stacked Brick Wall
            {
                auto cube = m_scene->CreateGameObject();
                auto meshRenderer = cube->AddComponent<MeshRenderer>();
                meshRenderer->SetMesh(MeshManager::Get().GetMesh("Cube"));
                cube->SetName("Quad_Right");

                auto mtrl = MaterialManager::Get().GetMaterial("Stacked_Brick_Wall");
                meshRenderer->SetMaterial(mtrl);

                auto transform = cube->GetTransform();
                transform->SetPosition(2.0f, 0.5f, 0.0f);
            }

            // Spherer
            {
                auto sphere = m_scene->CreateGameObject();
                auto meshRenderer = sphere->AddComponent<MeshRenderer>();
                meshRenderer->SetMesh(MeshManager::Get().GetMesh("Sphere"));
                sphere->SetName("Sphere");

                auto mtrl = MaterialManager::Get().GetMaterial("Marble_Cliff_06");
                meshRenderer->SetMaterial(mtrl);
                
                auto transform = sphere->GetTransform();
                transform->SetPosition(0.0f, 1.0f, -3.0f);
            }
        
            // Capsule
            {
                auto capsule = m_scene->CreateGameObject();
                auto meshRenderer = capsule->AddComponent<MeshRenderer>();
                meshRenderer->SetMesh(MeshManager::Get().GetMesh("Capsule"));
                capsule->SetName("Capsule");

                auto mtrl = MaterialManager::Get().GetMaterial("Rust_Metal_05");
                meshRenderer->SetMaterial(mtrl);

                auto transform = capsule->GetTransform();
                transform->SetPosition(-3.5f, 1.0f, -3.0f);
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

            m_showInspectorMenu = false;
        }
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
            auto mtrl = MaterialManager::Get().CreateMaterial("Tiles106");
            mtrl->SetMap(eMapType::Albedo, "Assets/Textures/Tiles106_1K-PNG/Tiles106_1K-PNG_Color.png");
            mtrl->SetMap(eMapType::Normal, "Assets/Textures/Tiles106_1K-PNG/Tiles106_1K-PNG_NormalDX.png");
            mtrl->SetMap(eMapType::Roughness, "Assets/Textures/Tiles106_1K-PNG/Tiles106_1K-PNG_Roughness.png");
            mtrl->SetMap(eMapType::Displacement, "Assets/Textures/Tiles106_1K-PNG/Tiles106_1K-PNG_Displacement.png");
            
            mtrl = MaterialManager::Get().CreateMaterial("Metal_Plate");
            mtrl->SetMap(eMapType::Albedo, "Assets/Textures/metal_plate_1k/metal_plate_diff_1k.png");
            mtrl->SetMap(eMapType::Normal, "Assets/Textures/metal_plate_1k/metal_plate_nor_dx_1k.png");
            mtrl->SetMap(eMapType::ORM, "Assets/Textures/metal_plate_1k/metal_plate_arm_1k.png");
            mtrl->SetMap(eMapType::Displacement, "Assets/Textures/metal_plate_1k/metal_plate_disp_1k.png");
            
            mtrl = MaterialManager::Get().CreateMaterial("Rusty_Metal_Grid");
            mtrl->SetMap(eMapType::Albedo, "Assets/Textures/rusty_metal_grid_1k/rusty_metal_grid_diff_1k.png");
            mtrl->SetMap(eMapType::Normal, "Assets/Textures/rusty_metal_grid_1k/rusty_metal_grid_nor_dx_1k.png");
            mtrl->SetMap(eMapType::ORM, "Assets/Textures/rusty_metal_grid_1k/rusty_metal_grid_arm_1k.png");
            mtrl->SetMap(eMapType::Displacement, "Assets/Textures/rusty_metal_grid_1k/rusty_metal_grid_disp_1k.png");
            
            mtrl = MaterialManager::Get().CreateMaterial("Stacked_Brick_Wall");
            mtrl->SetMap(eMapType::Albedo, "Assets/Textures/stacked_brick_wall_1k/stacked_brick_wall_diff_1k.png");
            mtrl->SetMap(eMapType::Normal, "Assets/Textures/stacked_brick_wall_1k/stacked_brick_wall_nor_dx_1k.png");
            mtrl->SetMap(eMapType::Displacement, "Assets/Textures/stacked_brick_wall_1k/stacked_brick_wall_disp_1k.png");
            
            mtrl = MaterialManager::Get().CreateMaterial("Marble_Cliff_06");
            mtrl->SetMap(eMapType::Albedo, "Assets/Textures/marble_cliff_06_1k/marble_cliff_06_diff_1k.png");
            mtrl->SetMap(eMapType::Normal, "Assets/Textures/marble_cliff_06_1k/marble_cliff_06_nor_dx_1k.png");
            mtrl->SetMap(eMapType::Roughness, "Assets/Textures/marble_cliff_06_1k/marble_cliff_06_rough_1k.png");
            mtrl->SetMap(eMapType::Displacement, "Assets/Textures/marble_cliff_06_1k/marble_cliff_06_disp_1k.png");
            
            mtrl = MaterialManager::Get().CreateMaterial("Rust_Metal_05");
            mtrl->SetMap(eMapType::Albedo, "Assets/Textures/rusty_metal_05_1k/rusty_metal_05_diff_1k.png");
            mtrl->SetMap(eMapType::Normal, "Assets/Textures/rusty_metal_05_1k/rusty_metal_05_nor_dx_1k.png");
            mtrl->SetMap(eMapType::ORM, "Assets/Textures/rusty_metal_05_1k/rusty_metal_05_arm_1k.png");
            mtrl->SetMap(eMapType::Displacement, "Assets/Textures/rusty_metal_05_1k/rusty_metal_05_disp_1k.png");
        }
    }
}