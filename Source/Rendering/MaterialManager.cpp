#include "pch.h"
#include "MaterialManager.h"

namespace Dive
{
	void MaterialManager::Initialize()
	{
		SetDirectory("Assets/Materials");
		CreateMaterial("Standard");
	}

	void MaterialManager::SaveToFiles()
	{
		if (!std::filesystem::exists(m_directory))
		{
			std::filesystem::create_directories(m_directory);
		}

		for (auto mat : m_materialList)
		{
			std::filesystem::path filepath = m_directory / mat->GetName();

			if (!mat->SaveToFile(filepath))
				spdlog::error("머티리얼 저장 실패: {}", filepath.string());
		}
	}

	Material* MaterialManager::LoadFromFile(const std::filesystem::path& filepath)
	{
		auto material = CreateMaterial(filepath.filename().string());
		if (!material->LoadFromFile(filepath))
		{
			spdlog::warn("머티리얼 파일 로드 실패: {}", filepath.string());
			return nullptr;
		}

		return material;
	}

	Material* MaterialManager::CreateMaterial(const std::string& name)
	{
		auto it = m_materials.find(name);
		if (it != m_materials.end())
		{
			spdlog::warn("동일한 이름의 머티리얼이 이미 존재: {}", name);
			return it->second.get();
		}

		auto material = std::make_unique<Material>();
		Material* rawPtr = material.get();

		m_materials[name] = std::move(material);
		m_materials[name]->SetName(name);
		m_materialList.push_back(rawPtr);

		return rawPtr;
	}

	Material* MaterialManager::GeStandard() const
	{
		return GetMaterial("Standard");
	}

	Material* MaterialManager::GetMaterial(const std::string& name) const
	{
		auto it = m_materials.find(name);
		if (it != m_materials.end())
			return it->second.get();
		
		return GeStandard();
	}
}