#include "pch.h"
#include "MaterialManager.h"

namespace Dive
{
	void MaterialManager::Initialize()
	{
		auto defaultMat = CreateMaterial("Default");
		defaultMat->SetMap("Assets/Textures/no_texture.png", eMapType::Albedo);
		//defaultMat->SetMap("Assets/Textures/stone01.tga", eMapType::Albedo);
		//defaultMat->SetMap("Assets/Textures/normal01.tga", eMapType::Normal);
	}

	Material* MaterialManager::LoadFromFile(const std::string& path)
	{
		return nullptr;
	}

	Material* MaterialManager::CreateMaterial(const std::string& name)
	{
		auto it = m_materials.find(name);
		if (it != m_materials.end())
		{
			spdlog::warn("동일한 이름의 머티리얼이 이미 존재: {}", name);
			return nullptr;
		}

		m_materials[name] = std::move(std::make_unique<Material>());
		return m_materials[name].get();
	}

	Material* MaterialManager::GetMaterial(const std::string& name) const
	{
		auto it = m_materials.find(name);
		if (it != m_materials.end())
			return it->second.get();
		return nullptr;
	}
}