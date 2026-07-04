#pragma once
#include "Resource.h"
#include "Core/Types.h"

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

		void Bind(Graphics* graphics);
		
		const std::string& GetShaderProgramName() const { return m_shaderName; }
		std::shared_ptr<ShaderProgram> GetShaderProgram();
		void SetShader(const std::string& name);
		
		std::shared_ptr<Texture2D> GetTexture(eTextureMapType type) const;
		void SetTexture(eTextureMapType type, const std::filesystem::path& filepath);
		
		// 여기도 Color로 바꾸기?
		DirectX::XMFLOAT4 GetDiffuseColor() const { return m_diffuseColor; }
		void SetDiffuseColor(const DirectX::XMFLOAT4& color) { m_diffuseColor = color; }
		void SetDiffuseColor(float r, float g, float b, float a) { m_diffuseColor = { r, g, b, a }; }

		DirectX::XMFLOAT2 GetTiling() const { return m_tiling; }
		void SetTiling(float x, float y) { m_tiling = { x, y }; }

		DirectX::XMFLOAT2 GetOffset() const { return m_offset; }
		void SetOffset(float x, float y) { m_offset = { x, y }; }


	private:
		std::string m_shaderName = "DefaultLit";
		std::shared_ptr<ShaderProgram> m_shaderProgram;

		DirectX::XMFLOAT4 m_diffuseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		DirectX::XMFLOAT2 m_tiling = { 1.0f, 1.0f };
		DirectX::XMFLOAT2 m_offset = { 0.0f, 0.0f };

		std::unordered_map<eTextureMapType, std::filesystem::path> m_texturePaths;
		std::unordered_map<eTextureMapType, std::shared_ptr<Texture2D>> m_textures;
		
	};
}