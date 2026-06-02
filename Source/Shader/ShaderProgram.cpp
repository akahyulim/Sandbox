#include "pch.h"
#include "ShaderProgram.h"
#include "Graphics/Graphics.h"

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

	void ShaderProgram::Bind(Graphics* graphics)
	{
		//graphics->BindVertexShader(m_vs.get());
		//graphics->BindPixelShader(m_ps.get());
		//graphics->BindInputLayout(m_il.get());
	}
}