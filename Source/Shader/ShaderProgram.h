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

		void Bind(Graphics* graphics);

		std::shared_ptr<VertexShader> GetVertexShader() const { return m_vs; }
		std::shared_ptr<PixelShader> GetPixelShader() const { return m_ps; }
		std::shared_ptr<InputLayout> GetInputLayout() const { return m_il; }

		std::string GetName() const { return m_name; }

	private:
		std::string m_name;

		std::shared_ptr<VertexShader> m_vs;
		std::shared_ptr<PixelShader> m_ps;
		std::shared_ptr<InputLayout> m_il;
	};
}