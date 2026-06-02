#pragma once
#include <Windows.h>
#include <string>

namespace Dive
{
	class Window
	{
	public:
		static Window& GetInst()
		{
			static Window instance;
			return instance;
		}

		Window(const Window&) = delete;
		Window(Window&&) = delete;
		Window& operator=(const Window&) = delete;
		Window& operator=(Window&&) = delete;

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
		Window() = default;
		~Window() = default;

	private:
		HWND m_hWnd{};
		std::wstring m_title = L"Untitled";
	};
}