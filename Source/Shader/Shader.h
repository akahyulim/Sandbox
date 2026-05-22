#pragma once
#include <string>
#include <d3d11.h>
#include <wrl/client.h>

namespace Dive
{
	class Shader
	{
	public:
		virtual ~Shader() = default;

		std::string GetName() const { return m_name; }
		void SetName(const std::string& name) { m_name = name; }

	protected:
		std::string m_name;
	}; 

	class VertexShader : public Shader
	{
	public:
		~VertexShader() override;

		ID3D11VertexShader* GetShader() const { return m_shader.Get(); }
		ID3D11VertexShader** GetAddressOf() { return m_shader.GetAddressOf(); }

	private:
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_shader;
	};

	class PixelShader : public Shader
	{
	public:
		~PixelShader() override;

		ID3D11PixelShader* GetShader() const { return m_shader.Get(); }
		ID3D11PixelShader** GetAddressOf() { return m_shader.GetAddressOf(); }

	private:
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_shader;
	};
}
