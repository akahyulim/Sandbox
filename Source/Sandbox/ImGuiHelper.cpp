#include "pch.h"
#include "ImGuiHelper.h"

namespace Dive
{
	namespace ImGuiHelper
	{
        void DrawFloatControl(const std::string& label, float& value, float resetValue, float columnWidth)
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

        void DrawVec2Control(const std::string& label, DirectX::XMFLOAT2& values, float resetValue, float columnWidth,
            const std::string& a, const std::string& b)
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

        void DrawVec3Control(const std::string& label, DirectX::XMFLOAT3& values, float resetValue, float columnWidth)
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

        DirectX::XMFLOAT4 ConvertXMFloat4(const ImVec4& value)
        {
            return { value.x, value.y, value.z, value.w };
        }

        ImVec4 ConvertImVec4(const DirectX::XMFLOAT4& value)
        {
            return { value.x, value.y, value.z, value.w };
        }

        bool WorldToScreenPoint(const DirectX::XMVECTOR& worldPos, const DirectX::XMMATRIX& viewProj,
            const ImVec2& viewportPos, const ImVec2& viewportSize, float& outScreenX, float& outScreenY)
        {
            DirectX::XMVECTOR projected = DirectX::XMVector3TransformCoord(worldPos, viewProj);

            float ndcX = DirectX::XMVectorGetX(projected);
            float ndcY = DirectX::XMVectorGetY(projected);
            float ndcZ = DirectX::XMVectorGetZ(projected);

            if (ndcZ < 0.0f || ndcZ > 1.0f)
                return false;

            outScreenX = viewportPos.x + (1.0f + ndcX) * 0.5f * viewportSize.x;
            outScreenY = viewportPos.y + (1.0f - ndcY) * 0.5f * viewportSize.y;

            return true;
        }
	}
}