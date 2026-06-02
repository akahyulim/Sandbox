#include "pch.h"
#include "Material.h"
#include "Texture2D.h"
#include "Graphics/Graphics.h"
#include "ResourceManager.h"

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