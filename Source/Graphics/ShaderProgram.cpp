#include "pch.h"
#include "ShaderProgram.h"
#include "Graphics/Graphics.h"
#include "Graphics/Shader.h"
#include "Graphics/InputLayout.h"

namespace Dive
{
	ShaderProgram::ShaderProgram(VertexShader* vs, PixelShader* ps, InputLayout* il)
		: m_vs(vs)
		, m_ps(ps)
		, m_il(il)
	{
	}

	ShaderProgram::ShaderProgram(ComputeShader* cs)
		: m_vs(nullptr)
		, m_ps(nullptr)
		, m_cs(cs)
		, m_il(nullptr)
	{
	}

	ShaderProgram& ShaderProgram::SetVertexShader(VertexShader* vs)
	{
		m_vs = vs;
		return *this;
	}

	ShaderProgram& ShaderProgram::SetPixelShader(PixelShader* ps)
	{
		m_ps = ps;
		return *this;
	}

	ShaderProgram& ShaderProgram::SetComputeShader(ComputeShader* cs)
	{
		m_cs = cs;
		return *this;
	}

	ShaderProgram& ShaderProgram::SetInputLayout(InputLayout* il)
	{
		m_il = il;
		return *this;
	}

	void ShaderProgram::Bind(Graphics* graphics)
	{
		if (m_il) 
			graphics->SetInputLayout(m_il);
		if (m_vs) 
			graphics->SetVertexShader(m_vs);
		if (m_ps) 
			graphics->SetPixelShader(m_ps);
		if (m_cs) 
			graphics->SetComputeShader(m_cs);
	}

	void ShaderProgram::Unbind(Graphics* graphics)
	{
		if (m_il) 
			graphics->SetInputLayout(nullptr);
		if (m_vs) 
			graphics->SetVertexShader(nullptr);
		if (m_ps) 
			graphics->SetPixelShader(nullptr);
		if (m_cs) 
			graphics->SetComputeShader(nullptr);
	}
}