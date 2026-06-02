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

	enum class eTextureMapType
	{
		Diffuse,
		Normal,
		Specular,
		Count
	};

	struct MaterialConstantData
	{
		DirectX::XMFLOAT4 ambientColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		DirectX::XMFLOAT4 diffuseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		DirectX::XMFLOAT4 specularColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		float specularPower = 32.0f;
		float padding[3];
	};

	class Material : public Resource
	{
	public:
		Material() = default;
		virtual ~Material() override = default;

		virtual bool Deserialize(const std::vector<uint8_t>& fileBuffer, std::wstring_view extension) override;

		virtual bool Create(Graphics* graphics) override;

		void SetShader(const std::string& name) { m_shaderName = name; }
		const std::string& GetShader() const { return m_shaderName; }

		void SetConstantData(const MaterialConstantData& data) { m_data = data; }
		const MaterialConstantData& GetConstantData() const { return m_data; }

		void SetTexture(eTextureMapType type, const std::filesystem::path& filepath);
		std::shared_ptr<Texture2D> GetTexture(eTextureMapType type) const;

	private:
		std::unordered_map<eTextureMapType, std::filesystem::path> m_texturePaths;
		std::unordered_map<eTextureMapType, std::shared_ptr<Texture2D>> m_textures;
		std::string m_shaderName = "DefaultLit";
		MaterialConstantData m_data;
	};
}