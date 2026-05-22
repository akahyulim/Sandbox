#pragma once
#include <wrl/client.h>
#include <d3d11.h>

namespace Dive
{
	class InputLayout
	{
	public:
		InputLayout() = default;
		~InputLayout() = default;

		ID3D11InputLayout* GetLayout() const { return m_inputLayout.Get(); }
		ID3D11InputLayout** GetAddressOf() { return m_inputLayout.GetAddressOf(); }

	private:
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	};
}