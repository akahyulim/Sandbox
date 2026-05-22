#include "pch.h"
#include "Window.h"
#include "StringUtils.h"

namespace Dive
{
	namespace
	{
		constexpr LPCWCHAR WND_CLASS_NAME = L"Sandbox_Win";
		constexpr int DEFAULT_WIDTH = 1280;
		constexpr int DEFAULT_HEIGHT = 720;
	}

	Window* Window::s_instance = nullptr;

	Window* Window::GetInstance()
	{
		if (s_instance == nullptr)
			s_instance = new Window;

		return s_instance;
	}

	bool Window::Initialize()
	{
		HINSTANCE hInstance = ::GetModuleHandle(nullptr);

		WNDCLASSEX wc{};
		wc.style = 0;
		wc.hInstance = hInstance;
		wc.lpfnWndProc = ::DefWindowProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
		wc.hIconSm = wc.hIcon;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = WND_CLASS_NAME;
		wc.cbSize = sizeof(WNDCLASSEX);

		if (!::RegisterClassEx(&wc))
		{
			spdlog::error("윈도우 클래스 등록 실패");
			return false;
		}

		DWORD style = WS_OVERLAPPEDWINDOW;

		int posX = (GetSystemMetrics(SM_CXSCREEN) - DEFAULT_WIDTH) / 2;
		int posY = (GetSystemMetrics(SM_CYSCREEN) - DEFAULT_HEIGHT) / 2;

		m_hWnd = CreateWindowEx(
			WS_EX_DLGMODALFRAME,
			WND_CLASS_NAME,
			L"Sandbox",
			WS_OVERLAPPEDWINDOW,
			posX > 0 ? posX : 0,
			posY > 0 ? posY : 0,
			DEFAULT_WIDTH, DEFAULT_HEIGHT,
			NULL, NULL, hInstance, NULL
		);

		if (!m_hWnd)
		{
			spdlog::error("윈도우 생성 실패");
			return false;
		}

		SendMessage(m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)NULL);
		SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)NULL);

		SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

		Show();

		return true;
	}

	bool Window::Run()
	{
		MSG msg{};
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessageW(&msg);
		}

		return msg.message != WM_QUIT;
	}

	void Window::SetMessageCallback(LONG_PTR callBack) const
	{
		if (!SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, callBack))
		{
			spdlog::error("윈도우 콜백 교체 실패");
		}
	}

	bool Window::Resize(uint32_t width, uint32_t height) const
	{
		assert(m_hWnd);

		int posX = (::GetSystemMetrics(SM_CXSCREEN) - width) / 2;
		int posY = (::GetSystemMetrics(SM_CYSCREEN) - height) / 2;

		if (!::SetWindowPos(m_hWnd, NULL, posX, posY, width, height, SWP_NOZORDER | SWP_DRAWFRAME))
		{
			spdlog::error("윈도우 크기 변경 실패");
			return false;
		}

		return true;
	}

	uint32_t Window::GetWidth() const
	{
		assert(m_hWnd != 0);

		RECT rt{};
		::GetClientRect(m_hWnd, &rt);

		return static_cast<uint32_t>(rt.right - rt.left);
	}

	uint32_t Window::GetHeight() const
	{
		assert(m_hWnd != 0);

		RECT rt{};
		::GetClientRect(m_hWnd, &rt);

		return static_cast<uint32_t>(rt.bottom - rt.top);
	}

	void Window::Show() const
	{
		assert(m_hWnd != 0);
		::ShowWindow(m_hWnd, SW_SHOW);
		::SetForegroundWindow(m_hWnd);
		::SetFocus(m_hWnd);
	}

	void Window::Hide() const
	{
		assert(m_hWnd != 0);
		::ShowWindow(m_hWnd, SW_HIDE);
	}

	void Window::Close() const
	{
		assert(m_hWnd != 0);
		::PostQuitMessage(0);
	}

	bool Window::IsWindowed() const
	{
		assert(m_hWnd != 0);

		if ((GetWindowLong(m_hWnd, GWL_STYLE) & WS_POPUP))
			return false;

		RECT rt{};
		::GetClientRect(m_hWnd, &rt);

		return (::GetSystemMetrics(SM_CXSCREEN) != static_cast<int>(rt.right - rt.left)) ||
			(::GetSystemMetrics(SM_CYSCREEN) != static_cast<int>(rt.bottom - rt.top));
	}

	void Window::FullScreen() const
	{
		assert(m_hWnd != 0);

		if (IsWindowed())
		{
			SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP);
			SetWindowPos(m_hWnd, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
		}
	}

	void Window::SetWindowed(bool windowed) const
	{
		assert(m_hWnd != 0);

		LONG currentStyle = GetWindowLong(m_hWnd, GWL_STYLE);

		if (windowed)
		{
			LONG newStyle = WS_POPUP;
			if (currentStyle != newStyle)
			{
				SetWindowLong(m_hWnd, GWL_STYLE, newStyle);
				SetWindowPos(m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
		else
		{
			LONG newStyle = WS_OVERLAPPEDWINDOW;
			if (currentStyle != newStyle)
			{
				SetWindowLong(m_hWnd, GWL_STYLE, newStyle);
				SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
	}

	void Window::Minimize() const
	{
		assert(m_hWnd != 0);
		if (!::IsIconic(m_hWnd))
			::ShowWindow(m_hWnd, SW_MINIMIZE);
	}

	void Window::Maximize() const
	{
		assert(m_hWnd != 0);
		if (!::IsZoomed(m_hWnd))
			::ShowWindow(m_hWnd, SW_MAXIMIZE);
	}

	void Window::Restore() const
	{
		assert(m_hWnd != 0);
		if (::IsZoomed(m_hWnd))
			::ShowWindow(m_hWnd, SW_RESTORE);
	}

	bool Window::IsMaximize() const
	{
		assert(m_hWnd != 0);
		return ::IsZoomed(m_hWnd);
	}

	void Window::SetTitle(const std::string& title)
	{
		m_title = StringUtils::StringToWString(title);
		std::wstring newTitle = L"Sandbox - " + m_title;
		::SetWindowText(m_hWnd, newTitle.c_str());
	}

	std::string Window::GetTitle() const
	{
		return StringUtils::WStringToString(m_title);
	}
}