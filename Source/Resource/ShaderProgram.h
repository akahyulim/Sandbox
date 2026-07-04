#pragma once
#include <string>
#include <memory>

#include "Resource.h"

namespace Dive
{
	class VertexShader;
	class PixelShader;
	class InputLayout;

	class ShaderProgram : public Resource
	{
	public:
		ShaderProgram() = delete;
		ShaderProgram(std::shared_ptr<VertexShader> vs, std::shared_ptr<PixelShader> ps, std::shared_ptr<InputLayout> il,
			const std::string& name);
		~ShaderProgram() = default;

		ID3D11VertexShader* GetVertexShader() const;
		ID3D11PixelShader* GetPixelShader() const;
		ID3D11InputLayout* GetInputLayout() const;

	private:
		std::shared_ptr<VertexShader> m_vs;
		std::shared_ptr<PixelShader> m_ps;
		std::shared_ptr<InputLayout> m_il;
	};
}