#pragma once
#include <string>
#include <DirectXMath.h>
#include <imgui/imgui.h>

namespace Dive
{
	namespace ImGuiHelper
	{
        void DrawFloatControl(const std::string& label, float& value, float resetValue = 0.0f, float columnWidth = 100.0f);
        void DrawVec2Control(const std::string& label, DirectX::XMFLOAT2& values, float resetValue = 0.0f, float columnWidth = 100.0f,
            const std::string& a = "X", const std::string& b = "Y");
        void DrawVec3Control(const std::string& label, DirectX::XMFLOAT3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
      
        DirectX::XMFLOAT4 ConvertXMFloat4(const ImVec4& value);
        ImVec4 ConvertImVec4(const DirectX::XMFLOAT4& value);

        bool WorldToScreenPoint(const DirectX::XMVECTOR& worldPos, const DirectX::XMMATRIX& viewProj, const ImVec2& viewportPos, const ImVec2& viewportSize,
            float& outScreenX, float& outScreenY);
	}
}