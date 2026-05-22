#include "pch.h"
#include "Sandbox.h"
#include "Core/Window.h"
#include "Core/EventDispatcher.h"
#include "Graphics/Graphics.h"
#include "Renderer/Renderer.h"

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
                Dive::Window::GetInstance()->Close();
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
    }

    Sandbox::~Sandbox()
    {
    }

    bool Sandbox::Initialize()
    {
        Window::GetInstance()->Initialize();
        Window::GetInstance()->SetMessageCallback((LONG_PTR)SandboxMessageHandler);

        m_graphics = std::make_unique<Graphics>();
        m_graphics->Initialize(
            Window::GetInstance()->GetWindowHandle(),
            Window::GetInstance()->GetWidth(),
            Window::GetInstance()->GetHeight(),
            Window::GetInstance()->IsWindowed()
        );
        s_graphics = m_graphics.get();

        m_renderer = std::make_unique<Renderer>();
        m_renderer->Initialize(m_graphics.get());

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
            ImGui_ImplWin32_Init(Window::GetInstance()->GetWindowHandle());
            ImGui_ImplDX11_Init(m_graphics->GetDevice(), m_graphics->GetDeviceContext());
        }

        return true;
    }

    void Sandbox::Run()
    {
        while (Window::GetInstance()->Run())
        {
            // 3D 공간 렌더링 (Renderer 레이어)
            m_renderer->Render();

            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            // 텍스트 박스에 타이핑 중이 아닐 때만 단축키 가동 (방어 코드)
            if (!ImGui::IsAnyItemActive())
            {
                if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Q))
                {
                    Window::GetInstance()->Close();
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
                    }

                    if (ImGui::MenuItem("Open"))
                    {
                    }

                    ImGui::Separator();

                    if (ImGui::BeginMenu("3D Object"))
                    {
                        if (ImGui::MenuItem("Triangle", nullptr, nullptr))
                        {
                        }
                        if (ImGui::MenuItem("Quad", nullptr, nullptr))
                        {
                        }
                        if (ImGui::MenuItem("Plane", nullptr, nullptr))
                        {
                        }
                        if (ImGui::MenuItem("Cube", nullptr, nullptr))
                        {
                        }
                        if (ImGui::MenuItem("Sphere", nullptr, nullptr))
                        {
                        }
                        if (ImGui::MenuItem("Capsule", nullptr, nullptr))
                        {
                        }
                        ImGui::EndMenu();
                    }

                    if (ImGui::MenuItem("Export Model File"))
                    {

                    }
                    
                    if (ImGui::BeginMenu("Light"))
                    {
                        if (ImGui::MenuItem("Directional Light"))
                        {

                        }
                        if (ImGui::MenuItem("Point Light"))
                        {

                        }
                        if (ImGui::MenuItem("Spot Light"))
                        {

                        }

                        ImGui::EndMenu();
                    }

                    ImGui::Separator(); // 구분선

                    if (ImGui::MenuItem("Copy"))
                    {
                    }
                    if (ImGui::MenuItem("Paste"))
                    {
                    }
                    if (ImGui::MenuItem("Delete"))
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
                        Window::GetInstance()->Close();
                    }

                    ImGui::EndPopup();
                }
            }
            ImGui::End();

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