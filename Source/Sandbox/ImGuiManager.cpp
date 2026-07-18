#include "pch.h"
#include "ImGuiManager.h"
#include "Core/Window.h"
#include "Graphics/Graphics.h"

#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/ImGuizmo.h>

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT32 msg, WPARAM wParam, LPARAM lParam);

namespace Dive
{
	ImGuiManager::ImGuiManager(Graphics* graphics)
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
		ImGui_ImplWin32_Init(graphics->GetWindow()->GetWindowHandle());
		ImGui_ImplDX11_Init(graphics->GetDevice(), graphics->GetDeviceContext());
	}
	
	ImGuiManager::~ImGuiManager()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiManager::Begin() const
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}
	
	void ImGuiManager::End() const
	{
		ImGui::Render();
		if (m_visible)
		{
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}

		/*
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
		*/
	}
	
	void ImGuiManager::HandleWindowMessage(const WindowEventData& data) const
	{
		ImGui_ImplWin32_WndProcHandler(static_cast<HWND>(data.handle), data.msg, data.wParam, data.lParam);
	}
}