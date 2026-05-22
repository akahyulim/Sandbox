#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <filesystem>

#include "Core/Types.h"

namespace Dive
{
	class VertexShader;
	class PixelShader;
	class InputLayout;
	class ShaderProgram;
	class Graphics;

	class ShaderManager
	{
	public:
		ShaderManager() = default;
		~ShaderManager();

		bool Initialize(Graphics* graphics);

		std::shared_ptr<ShaderProgram> GetProgram(const std::string& name);
		const std::unordered_map<std::string, std::shared_ptr<ShaderProgram>>& GetAllPrograms() { return m_shaderPrograms; }

	private:
		bool createVertexShaderAndInputLayout(Graphics* graphics, const std::filesystem::path& path, eInputLayout type);
		bool createPixelShader(Graphics* graphics, const std::filesystem::path& path);
		bool createShaderProgram(const std::string& vs, const std::string& ps, const std::string& name);

	private:
		std::unordered_map<std::string, std::pair<std::shared_ptr<VertexShader>, std::shared_ptr<InputLayout>>> m_vertexShaders;
		std::unordered_map<std::string, std::shared_ptr<PixelShader>> m_pixelShaders;
		std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> m_shaderPrograms;
	};
}
