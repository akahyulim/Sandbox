#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <filesystem>

#include "Core/Types.h"
#include "Graphics/Shader.h"
#include "Graphics/InputLayout.h"
#include "Graphics/ShaderProgram.h"

namespace Dive
{
	enum class eShaderPrograms : uint8_t
	{
		Unlit,
		LegacyLit,
		PbsLit,
		Deferred,
		Skybox,
		Resolve,
		Count
	};

	class Graphics;

	class ShaderManager
	{
	public:
		static ShaderManager& GetInst()
		{
			static ShaderManager instance;
			return instance;
		}

		ShaderManager(const ShaderManager&) = delete;
		ShaderManager(ShaderManager&&) = default;

		ShaderManager& operator=(const ShaderManager&) = delete;
		ShaderManager& operator=(ShaderManager&&) = default;

		bool Initialize(Graphics* graphics);

		ShaderProgram* GetShaderProgram(eShaderPrograms sp);

	private:
		ShaderManager() = default;
		~ShaderManager();

		bool createVertexShaderAndInputLayout(Graphics* graphics, const std::filesystem::path& path, eInputLayout type);
		bool createPixelShader(Graphics* graphics, const std::filesystem::path& path);
		bool createShaderProgram(const std::string& vsName, const std::string& psName, eShaderPrograms sp);

	private:
		std::unordered_map<std::string, std::unique_ptr<VertexShader>> m_vss;
		std::unordered_map<std::string, std::unique_ptr<PixelShader>> m_pss;
		std::unordered_map<std::string, std::unique_ptr<InputLayout>> m_ils;
		std::unordered_map<eShaderPrograms, std::unique_ptr<ShaderProgram>> m_shaderPrograms;
	};
}
