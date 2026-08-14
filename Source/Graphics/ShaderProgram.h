#pragma once
#include <string>

namespace Dive
{
	class VertexShader;
	class PixelShader;
	class InputLayout;
	class Graphics;

	class ShaderProgram
	{
	public:
		ShaderProgram(VertexShader* vs, PixelShader* ps, InputLayout* il);
		~ShaderProgram() = default;

		ShaderProgram& SetVertexShader(VertexShader* vs);
		ShaderProgram& SetPixelShader(PixelShader* ps);
		ShaderProgram& SetInputLayout(InputLayout* il);
		
		void Bind(Graphics* graphics);
		void Unbind(Graphics* graphics);

	private:
		VertexShader* m_vs = nullptr;
		PixelShader* m_ps = nullptr;
		InputLayout* m_il = nullptr;
	};
}