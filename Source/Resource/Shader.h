#pragma once
#include <d3d11.h>
#include <wrl/client.h>

#include "Resource.h"

namespace Dive
{
	class Shader : public Resource
	{
	public:
		virtual ~Shader() = default;
	}; 

	class VertexShader : public Shader
	{
	public:
		~VertexShader() override;

		ID3D11VertexShader* GetShader() const { return m_shader.Get(); }
		void SetShader(ID3D11VertexShader* shader) { m_shader = shader; }

		ID3D11VertexShader** GetAddressOf() { return m_shader.GetAddressOf(); }

		ID3D11InputLayout* GetInputLayout() const { return m_il.Get(); }
		void SetInputLayout(ID3D11InputLayout* il) { m_il = il; }

	private:
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_shader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_il;
	};

	class PixelShader : public Shader
	{
	public:
		~PixelShader() override;

		ID3D11PixelShader* GetShader() const { return m_shader.Get(); }
		void SetShader(ID3D11PixelShader* shader) { m_shader = shader; }

		ID3D11PixelShader** GetAddressOf() { return m_shader.GetAddressOf(); }

	private:
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_shader;
	};
}
