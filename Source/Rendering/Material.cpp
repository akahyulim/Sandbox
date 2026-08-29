#include "pch.h"
#include "Material.h"
#include "Graphics/Graphics.h"

namespace Dive
{
	Material::Material()
	{
		for (uint32_t i = 0; i < m_maps.size(); ++i)
		{
			m_maps[i] = INVALID_TEXTURE_HANDLE;
		}
	}

	ID3D11ShaderResourceView* Material::GetMap(eMapType type) const
	{
		if(type >= eMapType::Count)
			return nullptr;

		return TextureManager::Get().GetTextureView(m_maps[static_cast<size_t>(type)]);
	}

	void Material::SetMap(const std::string& path, eMapType type)
	{
		size_t index = static_cast<size_t>(type);

		m_maps[index] =  TextureManager::Get().LoadTexture(path);

		if (m_maps[index] != INVALID_TEXTURE_HANDLE)
			m_flags |= (1U << index);
		else
			m_flags &= ~(1U << index);
	}

	void Material::Bind(Graphics* graphics)
	{
		for(uint32_t i = 0; i < m_maps.size(); ++i)
		{
			auto handle = m_maps[i];
			if (handle != INVALID_TEXTURE_HANDLE)
			{
				auto srv = TextureManager::Get().GetTextureView(handle);

				if (srv)
					graphics->SetShaderResourceView(eShaderStage::PS, i, &srv);
			}
		}
	}
}
