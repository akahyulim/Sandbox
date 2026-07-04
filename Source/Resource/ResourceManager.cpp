#include "pch.h"
#include "ResourceManager.h"
#include "ShaderManager.h"
#include "Graphics/Geometry.h"

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

		// PipelineStates
		{
			auto opaque = std::make_shared<DvPipelineState>();
			opaque->topology = ePrimitiveTopology::TriangleList;
			// ShaderMahager 초기화가 먼저 수행되어야 한다.
			// 차라리 이름으로 저장하는 편이 나으려나...
			// => ShaderManager를 ResourceManager로 통합하라고 한다.
			// => 이 부분을 수행하자. ShaderProgram이 Resource를 상속해야 한다고 한다.
			opaque->shaderProgram = ShaderManager::GetInst().GetProgram("DefaultLit");
			opaque->depthStencilState = eDepthStencilState::DepthReadWrite;
			opaque->rasterizerState = eRasterizerState::FillSolid_CullBack;
			opaque->blendState = eBlendState::AlphaEnabled;
		}

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

		StaticGeometryData data;
		std::string name;

		switch (type)
		{
		case ePresetType::Triangle:
			data = Preset::GenerateTriangle();
			name = "Triangle";
			break;
		case ePresetType::Quad:
			data = Preset::GenerateQuad();
			name = "Quad";
			break;
		case ePresetType::Plane:
			data = Preset::GeneratePlane();
			name = "Plane";
			break;
		case ePresetType::Cube:
			data = Preset::GenerateCube();
			name = "Cube";
			break;
		case ePresetType::Sphere:
			data = Preset::GenerateSphere();
			name = "Sphere";
			break;
		case ePresetType::Capsule:
			data = Preset::GenerateCapsule();
			name = "Capsule";
			break;
		}

		auto presetMesh = m_graphics->CreateStaticMesh(data);
		if (presetMesh)
		{
			presetMesh->SetName(name);
			m_presetMeshes[type] = presetMesh;
		}
		return presetMesh;
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