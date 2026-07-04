#pragma once
#include <wrl/client.h>
#include <d3d11.h>

namespace Dive
{
	// 이건 Graphics에 캐싱하라고 한다.
	// 랩핑 클래스가 필요없을 수도 있다.
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