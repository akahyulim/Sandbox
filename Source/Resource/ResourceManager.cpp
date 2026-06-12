#include "pch.h"
#include "ResourceManager.h"
#include "Graphics/Graphics.h"

namespace Dive
{
	ResourceManager::ResourceManager()
		: m_resourcePath(std::filesystem::current_path())
	{
		spdlog::info("생성");
	}

	ResourceManager::~ResourceManager()
	{
		Clear();
		spdlog::info("종료");
	}
	
	bool ResourceManager::Initialize(Graphics* graphics)
	{
		assert(graphics);
		m_graphics = graphics;

		// Default Material
		auto defaultMat = std::make_shared<Material>();
		defaultMat->SetShader("LegacyLit");
		defaultMat->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
		Register("Default_Material", defaultMat);

		// Preset Meshes

		spdlog::info("초기화 성공");

		return true;
	}

	void ResourceManager::Clear()
	{
		m_resources.clear();
		spdlog::info("클리어");
	}

	std::shared_ptr<StaticMesh> ResourceManager::GetPresetMesh(ePresetType type)
	{
		if (m_presetMeshes.contains(type))
			return m_presetMeshes[type];

		auto mesh = std::make_shared<StaticMesh>();
		std::vector<StaticVertex> vertices;
		std::vector<uint32_t> indices;
		std::string name = "";

		switch (type)
		{
		case ePresetType::Triangle:
			Preset::GenerateTriangle(vertices, indices);
			name = "Triangle";
			break;
		case ePresetType::Quad:
			Preset::GenerateQuad(vertices, indices);
			name = "Quad";
			break;
		case ePresetType::Plane:
			Preset::GeneratePlane(vertices, indices);
			name = "Plane";
			break;
		case ePresetType::Cube:
			Preset::GenerateCube(vertices, indices);
			name = "Cube";
			break;
		case ePresetType::Sphere:
			Preset::GenerateSphere(vertices, indices);
			name = "Sphere";
			break;
		case ePresetType::Capsule:
			Preset::GenerateCapsule(vertices, indices);
			name = "Capsule";
			break;
		}

		mesh->SetVertices(vertices);
		mesh->SetIndices(indices);
		mesh->SetName(name);

		if (!mesh->Create(m_graphics))
		{
			spdlog::error("ResourceManager::GetPresetMesh - 버퍼 생성 실패: {}", name);
			return nullptr;
		}

		return mesh;
	}
	
	void ResourceManager::Unload(const std::shared_ptr<Resource>& resource)
	{
		if (!resource)
		{
			spdlog::warn("ResourceManager::Unload - 잘못된 리소스 객체 전달");
			return;
		}

		std::string key = resource->GetFilepath().string();

		Unload(key);
	}

	void ResourceManager::Unload(const std::string& key)
	{
		if (key.empty())
		{
			spdlog::warn("ResourceManager::Unload -  잘못된 리소스 키 전달: {}", key);
			return;
		}

		auto it = m_resources.find(key);
		if (it != m_resources.end())
			m_resources.erase(it);
	}

	std::filesystem::path ResourceManager::GetResourcePath() const
	{ 
		return m_resourcePath; 
	}
	
	void ResourceManager::SetResourcePath(const std::filesystem::path& path)
	{
		if (path.empty()) 
		{
			m_resourcePath = std::filesystem::current_path().generic_string();
			return;
		}

		try
		{
			auto absPath = std::filesystem::absolute(path);

			if (!std::filesystem::exists(absPath))
			{
				std::filesystem::create_directories(absPath);
			}
			m_resourcePath = absPath;
		}
		catch (const std::filesystem::filesystem_error e)
		{
			spdlog::error("ResourceManager::SetResourcePath - 파일 시스템 에러: {}", e.what());
			return;
		}
		catch(const std::exception e)
		{
			spdlog::error("ResourceManager::SetResourcePath - 예외 처리 발생: {}", e.what());
			return;
		}
	}
}