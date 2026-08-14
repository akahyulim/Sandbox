#pragma once
#include <wrl/client.h>
#include <d3d11.h>
#include <DXGI1_3.h>

namespace Dive
{
	enum class eInputLayout : uint8_t
	{
		None = 0,
		Position,
		Unlit,
		Lit,
		Skinned
	};

	class Graphics;

	class InputLayout
	{
	public:
		InputLayout(Graphics* graphics, eInputLayout type, ID3DBlob* code);
		~InputLayout();

		operator ID3D11InputLayout* () const { return m_il.Get(); }

	private:
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_il;
	};
}