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
	
	void ResourceManager::Register(const std::shared_ptr<Resource>& resource)
	{
		auto& filepath = resource->GetFilepath();
		auto key = filepath.generic_string();
		auto it = m_resources.find(key);
		if (it != m_resources.end())
		{
			spdlog::warn("[::Register] 이미 등록된 리소스: {}", filepath.string());
			return;
		}

		m_resources[key] = resource;
	}
	
	void ResourceManager::Unload(const std::shared_ptr<Resource>& resource)
	{
		if (!resource)
		{
			spdlog::warn("[::Unload] 잘못된 리소스 객체 전달");
			return;
		}

		auto key = resource->GetFilepath().generic_string();
		if (key.empty())
		{
			spdlog::warn("[::Unload] 리소스 Key 불일치");
			return;
		}

		auto it = m_resources.find(key);
		if (it != m_resources.end())
			m_resources.erase(it);
	}

	void ResourceManager::Unload(const std::filesystem::path& filepath)
	{
		if (filepath.empty())
		{
			spdlog::warn("[::Unload] 잘못된 리소스 파일 경로 전달");
			return;
		}

		auto key = filepath.generic_string();
		if (key.empty())
		{
			spdlog::warn("[::Unload] 리소스 Key 불일치");
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
			spdlog::error("[::SetResourcePath] 파일 시스템 에러: {}", e.what());
			return;
		}
		catch(const std::exception e)
		{
			spdlog::error("[::SetResourcePath] 예외 처리 발생: {}", e.what());
			return;
		}
	}
}