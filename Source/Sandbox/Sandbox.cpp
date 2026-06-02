#include "pch.h"
#include "Sandbox.h"
#include "Core/Window.h"
#include "Core/EventDispatcher.h"
#include "Core/Timer.h"
#include "Graphics/Graphics.h"
#include "Renderer/Renderer.h"
#include "Scene/Scene.h"
#include "Scene/Components/Camera.h"
#include "Scene/Components/Transform.h"
#include "Shader/ShaderManager.h"
#include "Resource/ResourceManager.h"

#include "Resource/Texture2D.h" // test

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT32 msg, WPARAM wParam, LPARAM lParam);

namespace Dive
{
    namespace
    {
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

        // scene 초기화
        {
            m_scene->SetClearColor(Color::LightSkyBlue);
            auto* mainCamera = m_scene->GetMainCamera();
            auto* transform = mainCamera->GetTransform();
            transform->SetPosition(0.0f, 0.0f, -5.0f);
            auto* cameraCom = mainCamera->GetComponent<Camera>();
            cameraCom->SetViewport(0.0f, 0.0f, (float)m_graphics->GetWidth(), (float)m_graphics->GetHeight());
        }

        {
            if (!ResourceManager::GetInst().Initialize(m_graphics.get()))
                return false;

            auto tex = ResourceManager::GetInst().Load<Texture2D>("Assets/Textures/DokeV.jpeg");
        }

        m_timer->Start();

        return true;
    }

    void Sandbox::Run()
    {
        while (Window::GetInst().Run())
        {
            m_timer->Tick();

            m_scene->Update(m_timer->GetDeltaTimeMS());

            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            // 텍스트 박스에 타이핑 중이 아닐 때만 단축키 가동 (방어 코드)
            if (!ImGui::IsAnyItemActive())
            {
                if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Q))
                {
                    Window::GetInst().Close();
                }

                // 나중에 Ctrl+S(저장), Ctrl+C(복사) 등도 여기에 줄줄이 얹으시면 됩니다.
            }

            // 💡 [핵심] 현재 메인 윈도우 창의 위치와 크기를 그대로 가져옵니다.
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);

            // 창의 외각선, 타이틀바, 크기 조절, 스크롤바 등을 전부 무력화하는 플래그 설정
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

                    if (ImGui::BeginMenu("Ground"))
                    {
                        if (ImGui::MenuItem("Grid"))
                        {

                        }
                        if (ImGui::MenuItem("Terrain", nullptr, nullptr, false))
                        {

                        }

                        ImGui::EndMenu();
                    }

                    if (ImGui::BeginMenu("3D Object"))
                    {
                        if (ImGui::MenuItem("Triangle", nullptr, nullptr, m_scene != nullptr))
                        {
                            m_scene->AddPresetObject(ePresetType::Triangle);
                        }
                        if (ImGui::MenuItem("Quad", nullptr, nullptr, m_scene != nullptr))
                        {
                            m_scene->AddPresetObject(ePresetType::Quad);
                        }
                        if (ImGui::MenuItem("Plane", nullptr, nullptr, m_scene != nullptr))
                        {
                            m_scene->AddPresetObject(ePresetType::Plane);
                        }
                        if (ImGui::MenuItem("Cube", nullptr, nullptr, m_scene != nullptr))
                        {
                            m_scene->AddPresetObject(ePresetType::Cube);
                        }
                        if (ImGui::MenuItem("Sphere", nullptr, nullptr, m_scene != nullptr))
                        {
                            m_scene->AddPresetObject(ePresetType::Sphere);
                        }
                        if (ImGui::MenuItem("Capsule", nullptr, nullptr, m_scene != nullptr))
                        {
                            m_scene->AddPresetObject(ePresetType::Capsule);
                        }
                        if (ImGui::MenuItem("Model", nullptr, nullptr, m_scene != nullptr))
                        {

                        }
                        ImGui::EndMenu();
                    }

                    ImGui::Separator(); // 구분선

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

            // 3D 공간 렌더링 (Renderer 레이어)
            m_renderer->Render(m_scene.get());

            m_graphics->BindMainRenderTarget();

            // 4. UI 출력 및 Present (이전과 동일)
            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            m_graphics->Present();
        }

        // 5. 해제 루틴
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
}