#pragma once
#include <memory>
#include <filesystem>
#include <map>
#include <spdlog/spdlog.h>

#include "Resource.h"
#include "StaticMesh.h"
#include "Preset.h"
#include "Texture2D.h"

namespace Dive
{
	class Graphics;
	
	class ResourceManager
	{
	public:
		static ResourceManager& GetInst()
		{
			static ResourceManager instance;
			return instance;
		}

		ResourceManager(const ResourceManager&) = delete;
		ResourceManager(ResourceManager&&) = delete;
		ResourceManager& operator=(const ResourceManager&) = delete;
		ResourceManager& operator=(ResourceManager&&) = delete;

		bool Initialize(Graphics* graphics);
		void Clear();

		std::shared_ptr<StaticMesh> GetPresetMesh(ePresetType type);

		template<class T>
		std::shared_ptr<T> Load(const std::filesystem::path& filepath)
		{
			assert(m_graphics);

			// 상대 경로만 사용
			// 절대 경로는 사용자에 따라 다를 수 있다.
			if (filepath.is_absolute())
			{
				spdlog::error("[::Load] 절대경로를 전달받았습니다: {}", filepath.string());
				return {};
			}

			if (filepath.empty())
				return {};

			std::filesystem::path key = filepath;
			auto it = m_resources.find(key);
			if (it != m_resources.end())
				return std::dynamic_pointer_cast<T>(it->second);

			auto resource = std::make_shared<T>();

			std::filesystem::path fullpath = std::filesystem::path(m_resourcePath) / filepath;

			if (!resource->LoadFromFile(fullpath)) 
				return {};
			if (!resource->Create(m_graphics)) 
				return {};
			resource->SetFilepath(filepath);

			m_resources[key] = resource;
			return resource;
		}

		template<class T>
		std::shared_ptr<T> Get(const std::filesystem::path& filepath)
		{
			auto it = m_resources.find(filepath);
			if (it != m_resources.end())
				return std::static_pointer_cast<T>(it->second);

			return nullptr;
		}

		// 무조건 리소스 파일이 존재하는 게 아니다.
		// 생성한 후 등록하는 과정도 필요하다.
		void Register(const std::shared_ptr<Resource>& resource);

		template<class T>
		std::vector<std::shared_ptr<T>> LoadAll(const std::filesystem::path& path)
		{
			std::vector<std::shared_ptr<T>> resources;

			if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path))
				return resources;

			for (const auto& entry : std::filesystem::directory_iterator(path))
			{
				if (!entry.is_regular_file()) continue;

				auto& filepath = entry.path();
				auto resource = Load<T>(filepath);
				if (resource)	resources.push_back(resource);
			}

			return resources;
		}

		void Unload(const std::shared_ptr<Resource>& resource);
		void Unload(const std::filesystem::path& filepath);

		std::filesystem::path GetResourcePath() const;
		void SetResourcePath(const std::filesystem::path& path);
		
	private:
		ResourceManager();
		~ResourceManager();

	private:
		Graphics* m_graphics = nullptr;

		std::filesystem::path m_resourcePath;

		std::unordered_map<std::filesystem::path, std::shared_ptr<Resource>> m_resources;
		std::unordered_map<ePresetType, std::shared_ptr<StaticMesh>> m_presetMeshes;
	};
}
