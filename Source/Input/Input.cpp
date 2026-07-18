#include "pch.h"
#include "Input.h"

using Microsoft::WRL::ComPtr;

namespace Dive
{
	Input::Input(HWND hWnd)
		: m_hWnd(hWnd)
	{
		if (!IsWindow(hWnd))
		{
			spdlog::error("[::Input] 잘못된 윈도우 핸들");
			return;
		}

		HINSTANCE hInst = ::GetModuleHandle(nullptr);
		auto hr = DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)m_directInput.GetAddressOf(), nullptr);
		if (FAILED(hr))
		{
			spdlog::error("[::Input] DirectInput8Create 실패: {}", ErrorUtils::ToVerbose(hr));
			return;
		}

		// 키보드 생성
		hr = m_directInput->CreateDevice(GUID_SysKeyboard, m_keyboard.GetAddressOf(), nullptr);
		if (FAILED(hr))
		{
			spdlog::error("[::Input] 키보드 CreateDevice 실패: {}", ErrorUtils::ToVerbose(hr));
			return;
		}

		hr = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
		if (FAILED(hr))
		{
			spdlog::error("[::Input] 키보드 SetDataFormat 실패: {}", ErrorUtils::ToVerbose(hr));
			return;
		}

		hr = m_keyboard->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		if (FAILED(hr))
		{
			spdlog::error("[::Input] 키보드 SetCooperativeLevel 실패: {}", ErrorUtils::ToVerbose(hr));
			return;
		}

		hr = m_keyboard->Acquire();
		if (FAILED(hr))
		{
			spdlog::error("[::Input] 키보드 Acquire 실패: {}", ErrorUtils::ToVerbose(hr));
			return;
		}

		// 마우스 생성
		hr = m_directInput->CreateDevice(GUID_SysMouse, m_mouse.GetAddressOf(), NULL);
		if (FAILED(hr))
		{
			spdlog::error("[::Input] 마우스 CreateDevice 실패: {}", ErrorUtils::ToVerbose(hr));
			return;
		}

		hr = m_mouse->SetDataFormat(&c_dfDIMouse);
		if (FAILED(hr))
		{
			spdlog::error("[::Input] 마우스 SetDataFormat 실패: {}", ErrorUtils::ToVerbose(hr));
			return;
		}

		hr = m_mouse->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		if (FAILED(hr))
		{
			spdlog::error("[::Input] 마우스 SetCooperativeLevel 실패: {}", ErrorUtils::ToVerbose(hr));
			return;
		}

		hr = m_mouse->Acquire();
		if (FAILED(hr))
		{
			spdlog::error("[::Input] 마우스 Acquire 실패: {}", ErrorUtils::ToVerbose(hr));
			return;
		}

		spdlog::info("초기화 성공");
	}

	Input::~Input()
	{
		if (m_mouse)
		{
			m_mouse->Unacquire();
			m_mouse.Reset();
		}

		if (m_keyboard)
		{
			m_keyboard->Unacquire();
			m_keyboard.Reset();
		}

		m_directInput.Reset();
	}

	bool Input::Initialize(HWND hWnd)
	{
		if (!IsWindow(hWnd))
		{
			spdlog::error("[::Initialize] 잘못된 윈도우 핸들");
			return false;
		}

		HINSTANCE hInst = ::GetModuleHandle(nullptr);
		auto hr = DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)m_directInput.GetAddressOf(), nullptr);
		if(FAILED(hr))
		{
			spdlog::error("[::Initialize] DirectInput8Create 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		// 키보드 생성
		hr = m_directInput->CreateDevice(GUID_SysKeyboard, m_keyboard.GetAddressOf(), nullptr);
		if (FAILED(hr))
		{
			spdlog::error("[::Initialize] 키보드 CreateDevice 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		hr = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
		if (FAILED(hr))
		{
			spdlog::error("[::Initialize] 키보드 SetDataFormat 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		hr = m_keyboard->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		if (FAILED(hr))
		{
			spdlog::error("[::Initialize] 키보드 SetCooperativeLevel 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		hr = m_keyboard->Acquire();
		if (FAILED(hr))
		{
			spdlog::error("[::Initialize] 키보드 Acquire 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		// 마우스 생성
		hr = m_directInput->CreateDevice(GUID_SysMouse, m_mouse.GetAddressOf(), NULL);
		if (FAILED(hr))
		{
			spdlog::error("[::Initialize] 마우스 CreateDevice 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		hr = m_mouse->SetDataFormat(&c_dfDIMouse);
		if (FAILED(hr))
		{
			spdlog::error("[::Initialize] 마우스 SetDataFormat 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		hr = m_mouse->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		if (FAILED(hr))
		{
			spdlog::error("[::Initialize] 마우스 SetCooperativeLevel 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		hr = m_mouse->Acquire();
		if (FAILED(hr))
		{
			spdlog::error("[::Initialize] 마우스 Acquire 실패: {}", ErrorUtils::ToVerbose(hr));
			return false;
		}

		m_hWnd = hWnd;

		spdlog::info("초기화 성공");
		return true;
	}

	void Input::Update()
	{
		memcpy(m_oldKeyStates, m_keyStates, sizeof(m_keyStates));
		memcpy(m_oldMouseButtons, m_mouseButtons, sizeof(m_mouseButtons));

		if (!ReadKeyboard()) return;
		if (!ReadMouse()) return;

		for (int i = 0; i < MAX_NUM_BUTTONS; ++i)
			m_mouseButtons[i] = m_mouseState.rgbButtons[i] ? 1 : 0;
	}

	bool Input::ReadKeyboard()
	{
		auto hr = m_keyboard->GetDeviceState(sizeof(m_keyStates), static_cast<LPVOID>(&m_keyStates));
		if (FAILED(hr))
		{
			if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
				m_keyboard->Acquire();
			else
			{
				spdlog::error("[::ReadKeyboard] 키보드 연결 실패: {}", ErrorUtils::ToVerbose(hr));
				return false;
			}
		}

		return true;
	}
	
	bool Input::ReadMouse()
	{
		auto hr = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), static_cast<LPVOID>(&m_mouseState));
		if (FAILED(hr))
		{
			if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
				m_mouse->Acquire();
			else
			{
				spdlog::error("[::ReadMouse] 마우스 연결 실패: {}", ErrorUtils::ToVerbose(hr));
				return false;
			}
		}

		return true;
	}

	bool Input::KeyState(int key)
	{
		return m_keyStates[key] & 0X80;
	}
	
	bool Input::KeyDown(int key)
	{
		return (m_keyStates[key] & 0X80) && !(m_oldKeyStates[key] & 0X80);
	}
	
	bool Input::KeyUp(int key)
	{
		return !(m_keyStates[key] & 0X80) && (m_oldKeyStates[key] & 0X80);
	}
	
	bool Input::KeyPress(int key)
	{
		return (m_keyStates[key] & 0X80) && (m_oldKeyStates[key] & 0X80);
	}

	bool Input::MouseButtonState(int btn)
	{
		return m_mouseButtons[btn] == 1;
	}
	
	bool Input::MouseButtonDown(int btn)
	{
		return (m_mouseButtons[btn] == 1) && !(m_oldMouseButtons[btn] == 1);
	}
	
	bool Input::MouseButtonUp(int btn)
	{
		return !(m_mouseButtons[btn] == 1) && (m_oldMouseButtons[btn] == 1);
	}
	
	bool Input::MouseButtonPress(int btn)
	{
		return (m_mouseButtons[btn] == 1) && (m_oldMouseButtons[btn] == 1);
	}
	
	DirectX::XMUINT2 Input::GetMousePosition()
	{
		return GetMousePosition(m_hWnd);
	}

	DirectX::XMUINT2 Input::GetMousePosition(HWND hWnd)
	{
		assert(hWnd);

		POINT point;
		GetCursorPos(&point);
		ScreenToClient(hWnd, &point);

		return DirectX::XMUINT2(static_cast<uint32_t>(point.x), static_cast<uint32_t>(point.y));
	}

	DirectX::XMFLOAT2 Input::GetMouseMoveDelta()
	{
		return { static_cast<float>(m_mouseState.lX), static_cast<float>(m_mouseState.lY) };
	}

	int Input::GetMouseWheelDelta()
	{
		return static_cast<int>(m_mouseState.lZ);
	}
}
