#pragma once

namespace Dive
{
	class Graphics;
	struct WindowEventData;

	class ImGuiManager
	{
	public:
		explicit ImGuiManager(Graphics* graphics);
		~ImGuiManager();

		void Begin() const;
		void End() const;
		void HandleWindowMessage(const WindowEventData& data) const;
		bool IsVisible() const { return m_visible; }

	private:
		bool m_visible = true;
	};
}