#pragma once
#include <string>

namespace Dive
{
	class VertexShader;
	class HullShader;
	class DomainShader;
	class PixelShader;
	class ComputeShader;
	class InputLayout;
	class Graphics;

	class ShaderProgram
	{
	public:
		ShaderProgram(VertexShader* vs, PixelShader* ps, InputLayout* il);
		ShaderProgram(ComputeShader* cs);
		~ShaderProgram() = default;

		ShaderProgram& SetVertexShader(VertexShader* vs);
		ShaderProgram& SetHullShader(HullShader* hs);
		ShaderProgram& SetDomainShader(DomainShader* ds);
		ShaderProgram& SetPixelShader(PixelShader* ps);
		ShaderProgram& SetComputeShader(ComputeShader* cs);
		ShaderProgram& SetInputLayout(InputLayout* il);
		
		void Bind(Graphics* graphics);
		void Unbind(Graphics* graphics);

	private:
		VertexShader* m_vs = nullptr;
		HullShader* m_hs = nullptr;
		DomainShader* m_ds = nullptr;
		PixelShader* m_ps = nullptr;
		ComputeShader* m_cs = nullptr;

		InputLayout* m_il = nullptr;
	};
}