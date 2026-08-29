#pragma once
#include <cstdint>
#include <Windows.h>
#include <dinput.h>
#include <DirectXMath.h>

namespace Dive
{
	inline constexpr uint16_t MAX_NUM_KEYS = 256;
	inline constexpr uint8_t MAX_NUM_BUTTONS = 4;

	class Input
	{
	public:
		static Input& Get()
		{
			static Input instance;
			return instance;
		}

		Input(HWND hWnd);
		~Input();

		Input(const Input&) = delete;
		Input(Input&&) = delete;
		Input& operator=(const Input&) = delete;
		Input& operator=(Input&&) = delete;

		bool Initialize(HWND hWnd);
		
		void Update();
		bool ReadKeyboard();
		bool ReadMouse();

		bool KeyState(int key);
		bool KeyDown(int key);
		bool KeyUp(int key);
		bool KeyPress(int key);

		bool MouseButtonState(int btn);
		bool MouseButtonDown(int btn);
		bool MouseButtonUp(int btn);
		bool MouseButtonPress(int btn);

		DirectX::XMUINT2 GetMousePosition();
		DirectX::XMUINT2 GetMousePosition(HWND hWnd);
		DirectX::XMFLOAT2 GetMouseMoveDelta();
		int GetMouseWheelDelta();

	private:
		Input() = default;
		//virtual ~Input();

	private:
		HWND m_hWnd{};
		Microsoft::WRL::ComPtr<IDirectInput8> m_directInput;
		Microsoft::WRL::ComPtr<IDirectInputDevice8> m_keyboard;
		Microsoft::WRL::ComPtr<IDirectInputDevice8> m_mouse;

		uint8_t m_keyStates[MAX_NUM_KEYS] = {};
		uint8_t m_oldKeyStates[MAX_NUM_KEYS] = {};

		DIMOUSESTATE m_mouseState = {};
		uint8_t m_mouseButtons[MAX_NUM_BUTTONS] = {};
		uint8_t m_oldMouseButtons[MAX_NUM_BUTTONS] = {};
	};
}
