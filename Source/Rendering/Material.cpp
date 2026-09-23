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

	Material::~Material() = default;

	ID3D11ShaderResourceView* Material::GetMap(eMapType type) const
	{
		if(type >= eMapType::Count)
			return nullptr;

		return TextureManager::Get().GetTextureView(m_maps[static_cast<size_t>(type)]);
	}

	void Material::SetMap(const std::string& path, eMapType type)
	{
		size_t slot = static_cast<size_t>(type);

		m_maps[slot] =  TextureManager::Get().LoadTexture(path);

		if (m_maps[slot] != INVALID_TEXTURE_HANDLE)
		{
			m_data.flags |= (1U << slot);

			if (type == eMapType::ORM)
			{
				m_data.roughnessFactor = 1.0f;
				m_data.metallicFactor = 1.0f;
			}
		}
		else
		{
			m_data.flags &= ~(1U << slot);

			if (type == eMapType::ORM)
			{
				m_data.roughnessFactor = 0.0f;
				m_data.metallicFactor = 0.0f;
			}
		}

		MarkDirty();
	}

	bool Material::HasMap(eMapType type) const
	{
		size_t slot = static_cast<size_t>(type);
		return m_maps[slot] != INVALID_TEXTURE_HANDLE;
	}

	void Material::SetBaseColor(const DirectX::XMFLOAT4& color)
	{
		auto current = m_data.baseColor;
		if (current.x != color.x || current.y != color.y ||
			current.z != color.z || current.w != color.w)
		{
			m_data.baseColor = color;
			MarkDirty();
		}
	}

	void Material::SetBaseColor(float r, float g, float b, float a)
	{
		SetBaseColor(DirectX::XMFLOAT4(r, g, b, a));
	}

	void Material::SetEmissiveFactor(const DirectX::XMFLOAT3& factor)
	{
		auto current = m_data.emissiveFactor;
		if (current.x != factor.x || current.y != factor.y || current.z != factor.z)
		{
			m_data.emissiveFactor = factor;
			MarkDirty();
		}
	}

	void Material::SetEmissiveFactor(float r, float g, float b)
	{
		SetEmissiveFactor(DirectX::XMFLOAT3(r, g, b));
	}

	void Material::SetRoughnessFactor(float factor)
	{
		if (m_data.roughnessFactor != factor)
		{
			m_data.roughnessFactor = factor;
			MarkDirty();
		}
	}

	void Material::SetTiling(const DirectX::XMFLOAT2& tiling)
	{
		auto current = m_data.tiling;
		if (current.x != tiling.x || current.y != tiling.y)
		{
			m_data.tiling = tiling;
			MarkDirty();
		}
	}

	void Material::SetTiling(float x, float y)
	{
		SetTiling(DirectX::XMFLOAT2(x, y));
	}

	void Material::SetOffset(const DirectX::XMFLOAT2& offset)
	{
		auto current = m_data.offset;
		if (current.x != offset.x || current.y != offset.y)
		{
			m_data.offset = offset;
			MarkDirty();
		}
	}

	void Material::SetOffset(float x, float y)
	{
		SetOffset(DirectX::XMFLOAT2(x, y));
	}

	void Material::SetMetallicFactor(float factor)
	{
		if (m_data.metallicFactor != factor)
		{
			m_data.metallicFactor = factor;
			MarkDirty();
		}
	}

	void Material::Bind(Graphics* graphics)
	{
		if (!m_cbuffer)
		{
			m_cbuffer = std::make_unique<ConstantBuffer<MaterialData>>(graphics);
		}

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

		m_cbuffer->Update(graphics, m_data);
		m_cbuffer->Bind(graphics, eShaderStage::PS, static_cast<uint32_t>(eConstantBuffer::Material));
	}
}
