#include "pch.h"
#include "Material.h"
#include "Texture2D.h"
#include "Graphics/Graphics.h"
#include "ResourceManager.h"
#include "Renderer/ShaderType.h"
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

		auto tex = m_textures[eTextureMapType::Diffuse];
		if (tex)
			graphics->BindTexture(tex);

		cbMaterial data{};
		if (tex)
			data.flags |= 1U;

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

		std::shared_ptr<Texture2D> tex = filepath.empty() ? nullptr : ResourceManager::GetInst().Get<Texture2D>(filepath);

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