#include "pch.h"
#include "Material.h"
#include "Texture2D.h"
#include "Graphics/Graphics.h"
#include "ResourceManager.h"
#include "Graphics/ShaderType.h"
#include "Shader/ShaderManager.h"
#include "Shader/ShaderProgram.h"

namespace Dive
{
	bool Material::Deserialize(const std::vector<uint8_t>& fileBuffer, std::wstring_view extension)
	{
		return false;
	}

	bool Material::Create(Graphics* graphics)
	{
		return false;
	}

	void Material::Bind(Graphics* graphics)
	{
		assert(graphics);

		auto& diffTex = m_textures[eTextureMapType::Diffuse];
		if (diffTex)
			graphics->BindTexture(static_cast<UINT>(eTextureMapType::Diffuse), diffTex);	// 슬롯을 전달해야 한다.
		auto& norTex = m_textures[eTextureMapType::Normal];
		if (norTex)
			graphics->BindTexture(static_cast<UINT>(eTextureMapType::Normal), norTex);

		cbMaterial data{};
		if (diffTex)
			data.flags |= 1U;
		if (norTex)
			data.flags |= 1U << 1;
		data.diffuseColor = m_diffuseColor;
		data.tiling = m_tiling;
		data.offset = m_offset;

		graphics->UpdateConstantBuffer(eCBufferSlot::Material, &data, sizeof(data));
		graphics->BindConstantBuffer(eCBufferSlot::Material);
	}

	void Material::SetShader(const std::string& name)
	{
		auto shaderProgram = ShaderManager::GetInst().GetProgram(name);
		if (shaderProgram == nullptr)
		{
			spdlog::error("Material::SetShader - 존재하지 않는 셰이더 프로그램: {}", name);			

			if (m_shaderProgram == nullptr)
			{
				m_shaderName = "DefaultLit";
				m_shaderProgram = ShaderManager::GetInst().GetProgram("DefaultLit");
			}
			return;
		}

		m_shaderName = name;
		m_shaderProgram = shaderProgram;
	}

	std::shared_ptr<ShaderProgram> Material::GetShaderProgram()
	{
		if (m_shaderProgram == nullptr)
		{
			m_shaderProgram = ShaderManager::GetInst().GetProgram(m_shaderName);
			if (m_shaderProgram == nullptr)
			{
				spdlog::error("Material::GetShaderProgram - 존재하지 않는 셰이더 프로그램: {}", m_shaderName);

				if (m_shaderProgram == nullptr)
				{
					m_shaderName = "DefaultLit";
					m_shaderProgram = ShaderManager::GetInst().GetProgram("DefaultLit");
				}
			}
		}

		return m_shaderProgram;
	}
	
	void Material::SetTexture(eTextureMapType type, const std::filesystem::path& filepath)
	{
		if (!filepath.empty() && !std::filesystem::exists(filepath))
		{
			spdlog::warn("Material::SetTextur - 존재하지 않는 텍스쳐 경로입니다: {}", filepath.string());
			
			m_texturePaths.erase(type);
			m_textures.erase(type);
			return;
		}

		std::shared_ptr<Texture2D> tex = filepath.empty() ? nullptr : ResourceManager::GetInst().GetFromFile<Texture2D>(filepath);

		if (tex != nullptr)
		{
			m_texturePaths[type] = filepath;
			m_textures[type] = tex;
		}
		else
		{
			m_texturePaths.erase(type);
			m_textures.erase(type);
		}
	}

	std::shared_ptr<Texture2D> Material::GetTexture(eTextureMapType type) const
	{
		auto it = m_textures.find(type);
		if (it != m_textures.end())
			return it->second;

		return nullptr;
	}
}