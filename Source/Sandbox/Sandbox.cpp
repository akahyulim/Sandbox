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

        m_skyCubemaps.emplace("Cloudy", TextureManager::Get().LoadCubemap(L"Assets/Textures/Skybox/cloudy_skybox.dds"));
        m_skyCubemaps.emplace("Sunset", TextureManager::Get().LoadCubemap(L"Assets/Textures/Skybox/sunsetcube1024.dds"));
        m_skyCubemaps.emplace("Desert", TextureManager::Get().LoadCubemap(L"Assets/Textures/Skybox/desertcube1024.dds"));
 
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
                        else if (m_showPropertiesMenu) m_showPropertiesMenu = false;
                        else if (m_selectedObject) setSelectedObject(nullptr);
                        else m_showQuitMenu = !m_showQuitMenu;
                    }

                    if (ImGui::IsKeyPressed(ImGuiKey_F1))
                    {
                        m_showEnviromentMenu = !m_showEnviromentMenu;
                    }

                    if (ImGui::IsKeyPressed(ImGuiKey_I) && m_selectedObject)
                    {
                        m_showPropertiesMenu = !m_showPropertiesMenu;
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
        showProperties();
        showQuit();
    }

    void Sandbox::drawCanvasContextMenu()
    {
        if (m_isSceneViewHovered && !ImGuizmo::IsUsing() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
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

                ImGui::MenuItem("Properties", "I", &m_showPropertiesMenu);
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
                    if (ImGui::MenuItem("Spot Light"))
                    {
                    }
                    if (ImGui::MenuItem("Point Light"))
                    {
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

        if (m_showPropertiesMenu) m_showPropertiesMenu = false;

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

                if (ImGui::RadioButton("Skybox", &currentSkyMode, 0))
                    renderer->SetSkyMode(eSkyMode::Skybox);
                ImGui::SameLine();
                if(ImGui::RadioButton("Uniform Color", &currentSkyMode, 1))
                    renderer->SetSkyMode(eSkyMode::UniformColor);

                if (currentSkyMode == 0)
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

            // 일단 Lighting부터 적용하자.
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

    void Sandbox::showProperties()
    {
        if (!m_showPropertiesMenu || !m_selectedObject)
            return;

        // 호출 순서때문에 이게 안먹힌다.
        //if (m_showEnviromentMenu) m_showEnviromentMenu = false;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos({ viewport->WorkSize.x - 350.0f, viewport->WorkPos.y });
        ImGui::SetNextWindowSize({ 350.0f, viewport->WorkSize.y });

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_HorizontalScrollbar;

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));

        if (ImGui::Begin("Properties", &m_showPropertiesMenu, flags))
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

            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::CollapsingHeader("Material"))
            {
                auto meshRenderer = m_selectedObject->GetComponent<MeshRenderer>();
                auto material = meshRenderer->GetMaterial();

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
                    const char* pFilter = "Texture Files (*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga)\0*.png;*.jpg;*.jped;*.dds;*.bmp;*.tga\0"
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
                    const char* pFilter = "Texture Files (*.png;*.jpg;*.jpeg;*.dds;*.bmp;*.tga)\0*.png;*.jpg;*.jped;*.dds;*.bmp;*.tga\0"
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

                // Metalic Map
                ImGui::PushID("MetalicMap");
                textureID = (ImTextureID)(material->GetMap(eMapType::Metallic) ?
                    material->GetMap(eMapType::Metallic) : 0);
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, 150.0f);
                if (ImGui::ImageButton("##Metalic", textureID, ImVec2(20, 20)))
                {
                }
                ImGui::SameLine();
                ImGui::Text("Metalic Map");
                ImGui::NextColumn();
                float metalicFactor = material->GetMetalicFactor();
                ImGui::SliderFloat("##MetalicFactor", &metalicFactor, 0.0f, 1.0f, "%.2f");
                material->SetMetalicFactor(metalicFactor);
                ImGui::Columns(1);
                ImGui::PopID();

                // Roughness Map
                ImGui::PushID("RoughnessMap");
                textureID = (ImTextureID)(material->GetMap(eMapType::Roughness) ?
                    material->GetMap(eMapType::Metallic) : 0);
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

                // Emissive Map
                ImGui::PushID("EmissiveMap");
                textureID = (ImTextureID)(material->GetMap(eMapType::Emissive) ?
                    material->GetMap(eMapType::Emissive) : 0);
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, 150.0f);
                if (ImGui::ImageButton("##Emissive", textureID, ImVec2(20, 20)))
                {
                }
                ImGui::SameLine();
                ImGui::Text("Emissive Map");
                ImGui::NextColumn();
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
        env.skyboxCubemap = m_skyCubemaps["Cloudy"];

        m_mainCamera = m_scene->GetCamera();

        // field
        {
            m_field = m_scene->CreateGameObject();
            m_field->SetName("field");
            auto meshRenderer = m_field->AddComponent<MeshRenderer>();
            meshRenderer->SetMesh(MeshManager::Get().GetMesh("Plane"));

            auto mtrl = MaterialManager::Get().CreateMaterial("Field");
            mtrl->SetMap("Assets/Textures/stone01.tga", eMapType::Albedo);
            mtrl->SetMap("Assets/Textures/normal01.tga", eMapType::Normal);
            meshRenderer->SetMaterial(mtrl);
        }

        // Cube
        {
            auto cube = m_scene->CreateGameObject();
            auto meshRenderer = cube->AddComponent<MeshRenderer>();
            meshRenderer->SetMesh(MeshManager::Get().GetMesh("Cube"));
            cube->SetName("Cube");

            auto mtrl = MaterialManager::Get().CreateMaterial("Cube");
            mtrl->SetBaseColor(1.0f, 0.0f, 0.0f, 1.0f);
            meshRenderer->SetMaterial(mtrl);

            cube->GetTransform()->SetPosition(-3.0f, 1.0f, 3.0f);
        }

        // Spherer
        {
            auto sphere = m_scene->CreateGameObject();
            auto meshRenderer = sphere->AddComponent<MeshRenderer>();
            meshRenderer->SetMesh(MeshManager::Get().GetMesh("Sphere"));
            sphere->SetName("Sphere");

            //auto mtrl = MaterialManager::Get().CreateMaterial("Sphere");
            //mtrl->SetMap("Assets/Textures/dokev.jpeg", eMapType::Albedo);
            //meshRenderer->SetMaterial(mtrl);

            sphere->GetTransform()->SetPosition(0.0f, 1.0f, 3.0f);
        }

        // Capsule
        {
            auto capsule = m_scene->CreateGameObject();
            auto meshRenderer = capsule->AddComponent<MeshRenderer>();
            meshRenderer->SetMesh(MeshManager::Get().GetMesh("Capsule"));
            capsule->SetName("Capsule");

            auto mtrl = MaterialManager::Get().CreateMaterial("Capsule");
            mtrl->SetBaseColor(0.0f, 0.0f, 1.0f, 1.0f);
            meshRenderer->SetMaterial(mtrl);

            capsule->GetTransform()->SetPosition(3.0f, 1.0f, 3.0f);
        }

        // Quad Left
        {
            auto quad = m_scene->CreateGameObject();
            auto meshRenderer = quad->AddComponent<MeshRenderer>();
            meshRenderer->SetMesh(MeshManager::Get().GetMesh("Quad"));
            quad->SetName("Quad_Left");

            auto mtrl = MaterialManager::Get().CreateMaterial("Quad_Left");
            mtrl->SetMap("Assets/Textures/dokev.jpeg", eMapType::Albedo);
            meshRenderer->SetMaterial(mtrl);

            quad->GetTransform()->SetScale({ 5.0f, 5.0f, 1.0f });
            quad->GetTransform()->SetPosition(-2.5f, 2.5f, 5.0f);
        }

        // Quad Right
        {
            auto quad = m_scene->CreateGameObject();
            auto meshRenderer = quad->AddComponent<MeshRenderer>();
            meshRenderer->SetMesh(MeshManager::Get().GetMesh("Quad"));
            quad->SetName("Quad_Right");

            auto mtrl = MaterialManager::Get().CreateMaterial("Quad_Right"); 
            mtrl->SetMap("Assets/Textures/dmc.jpg", eMapType::Albedo);
            meshRenderer->SetMaterial(mtrl);

            quad->GetTransform()->SetScale({ 5.0f, 5.0f, 1.0f });
            quad->GetTransform()->SetPosition(2.5f, 2.5f, 5.0f);
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

            m_showPropertiesMenu = false;
        }
    }
}