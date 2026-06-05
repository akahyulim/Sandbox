#pragma once
#include "Resource.h"

#include <DirectXMath.h>
#include <unordered_map>
#include <memory>
#include <filesystem>

namespace Dive
{
	class Graphics;
	class Texture2D;
	class ShaderProgram;

	enum class eTextureMapType
	{
		Diffuse,
		Normal,
		Specular,
		Count
	};

	class Material : public Resource
	{
	public:
		Material() = default;
		virtual ~Material() override = default;

		virtual bool Deserialize(const std::vector<uint8_t>& fileBuffer, std::wstring_view extension) override;

		virtual bool Create(Graphics* graphics) override;

		void Bind(Graphics* graphics);

		void SetShader(const std::string& name);
		const std::string& GetShaderProgramName() const { return m_shaderName; }
		std::shared_ptr<ShaderProgram> GetShaderProgram();

		void SetTexture(eTextureMapType type, const std::filesystem::path& filepath);
		std::shared_ptr<Texture2D> GetTexture(eTextureMapType type) const;

	private:
		std::unordered_map<eTextureMapType, std::filesystem::path> m_texturePaths;
		std::unordered_map<eTextureMapType, std::shared_ptr<Texture2D>> m_textures;
		std::string m_shaderName = "DefaultLit";
		std::shared_ptr<ShaderProgram> m_shaderProgram;
	};
}