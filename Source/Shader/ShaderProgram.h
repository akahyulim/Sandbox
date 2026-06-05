#pragma once
#include <string>
#include <memory>

namespace Dive
{
	class VertexShader;
	class PixelShader;
	class InputLayout;
	class Graphics;

	class ShaderProgram
	{
	public:
		ShaderProgram(std::shared_ptr<VertexShader> vs, std::shared_ptr<PixelShader> ps, std::shared_ptr<InputLayout> il,
			const std::string& name);
		~ShaderProgram() = default;

		ID3D11VertexShader* GetVertexShader() const;
		ID3D11PixelShader* GetPixelShader() const;
		ID3D11InputLayout* GetInputLayout() const;

		std::string GetName() const { return m_name; }

	private:
		std::string m_name;

		std::shared_ptr<VertexShader> m_vs;
		std::shared_ptr<PixelShader> m_ps;
		std::shared_ptr<InputLayout> m_il;
	};
}