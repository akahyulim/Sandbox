#include "pch.h"
#include "ShaderProgram.h"
#include "Graphics/Graphics.h"
#include "Shader.h"
#include "InputLayout.h"

namespace Dive
{
	ShaderProgram::ShaderProgram(
		std::shared_ptr<VertexShader> vs, 
		std::shared_ptr<PixelShader> ps, 
		std::shared_ptr<InputLayout> il,
		const std::string& name)
		: m_vs(vs)
		, m_ps(ps)
		, m_il(il)
		, m_name(name)
	{
	}

	ID3D11VertexShader* ShaderProgram::GetVertexShader() const
	{
		return m_vs ? m_vs->GetShader() : nullptr;
	}

	ID3D11PixelShader* ShaderProgram::GetPixelShader() const
	{
		return m_ps ? m_ps->GetShader() : nullptr;
	}
	
	ID3D11InputLayout* ShaderProgram::GetInputLayout() const
	{
		return m_il ? m_il->GetLayout() : nullptr;
	}
}