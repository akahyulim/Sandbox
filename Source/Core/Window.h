#pragma once
#include <Windows.h>
#include <string>

namespace Dive
{
	class Window
	{
	public:
		static Window* GetInstance();

		bool Initialize();

		bool Run();

		void SetMessageCallback(LONG_PTR callBack) const;

		bool Resize(uint32_t width, uint32_t height) const;
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;

		HWND GetWindowHandle() const { return m_hWnd; }

		void Show() const;
		void Hide() const;
		void Close() const;

		bool IsWindowed() const;
		void FullScreen() const;

		void SetWindowed(bool windowed = true) const;

		void Minimize() const;
		void Maximize() const;
		void Restore() const;
		bool IsMaximize() const;

		void SetTitle(const std::string& title);
		std::string GetTitle() const;

	private:
		static Window* s_instance;

		HWND m_hWnd{};
		std::wstring m_title = L"Untitled";
	};
}