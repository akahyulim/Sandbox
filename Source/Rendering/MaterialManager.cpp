#include "pch.h"
#include "MaterialManager.h"

namespace Dive
{
	void MaterialManager::Initialize()
	{
		m_default = std::make_unique<Material>();
		m_default->SetName("Default");
		m_default->SetBaseColor(1.0f, 1.0f, 1.0f, 1.0f);
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
			return it->second.get();
		}

		m_materials[name] = std::move(std::make_unique<Material>());
		m_materials[name]->SetName(name);
		return m_materials[name].get();
	}

	Material* MaterialManager::GetDefault() const
	{
		assert(m_default);
		return m_default.get();
	}

	Material* MaterialManager::GetMaterial(const std::string& name) const
	{
		auto it = m_materials.find(name);
		if (it != m_materials.end())
			return it->second.get();
		
		return GetDefault();
	}
}