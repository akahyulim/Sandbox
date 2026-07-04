#pragma once
#include <memory>
#include <filesystem>
#include <map>
#include <spdlog/spdlog.h>

#include "Resource.h"
#include "StaticMesh.h"
#include "Preset.h"
#include "Texture2D.h"
#include "Material.h"
#include "DvPipelineState.h"
#include "ResourceDataLoader.h"
#include "Graphics/Graphics.h"

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

		template<typename T>
		std::shared_ptr<T> Load(const std::filesystem::path& filepath);

		template<typename T>
		std::shared_ptr<T> Get(const std::string& key);
		template<typename T>
		std::shared_ptr<T> GetFromFile(const std::filesystem::path& filepath);

		template<typename T>
		bool Register(const std::string& key, std::shared_ptr<T> resource);

		template<typename T>
		std::vector<std::shared_ptr<T>> LoadAll(const std::filesystem::path& path);

		void Unload(const std::shared_ptr<Resource>& resource);
		void Unload(const std::string& key);

		std::filesystem::path GetResourcePath() const;
		void SetResourcePath(const std::filesystem::path& path);

	private:
		ResourceManager();
		~ResourceManager();

	private:
		Graphics* m_graphics = nullptr;

		std::filesystem::path m_resourcePath;

		std::unordered_map<std::string, std::shared_ptr<Resource>> m_resources;
		std::unordered_map<ePresetType, std::shared_ptr<StaticMesh>> m_presetMeshes;
	};

	template<>
	inline std::shared_ptr<Texture2D> ResourceManager::Load<Texture2D>(const std::filesystem::path& filepath)
	{
		assert(m_graphics);

		// 상대 경로만 사용
		// 절대 경로는 사용자에 따라 다를 수 있다.
		if (filepath.is_absolute())
		{
			spdlog::error("ResourceManager::Load - 절대경로를 전달받았습니다: {}", filepath.string());
			return nullptr;
		}

		if (filepath.empty())
			return nullptr;

		std::string key = filepath.string();
		auto it = m_resources.find(key);
		if (it != m_resources.end())
			return std::dynamic_pointer_cast<Texture2D>(it->second);

		std::filesystem::path fullpath = std::filesystem::path(m_resourcePath) / filepath;

		auto texData = ResourceDataLoader::LoadTextureData(fullpath);
		if (!texData) 
			return nullptr;

		auto texture = m_graphics->CreateTexture2D(texData->scratchImage.get(), texData->metaData.get());
		if (!texture) 
			return nullptr;

		texture->SetFilepath(filepath);
		m_resources[key] = texture;
		return texture;
	}

	template<typename T>
	std::shared_ptr<T> ResourceManager::Get(const std::string& key)
	{
		if (key.empty())
			return nullptr;

		auto it = m_resources.find(key);
		if (it != m_resources.end())
			return std::static_pointer_cast<T>(it->second);

		return nullptr;
	}

	template<typename T>
	std::shared_ptr<T> ResourceManager::GetFromFile(const std::filesystem::path& filepath)
	{
		if (filepath.empty())
			return nullptr;

		std::string key = filepath.string();

		return Get<T>(key);
	}

	template<typename T>
	bool ResourceManager::Register(const std::string& key, std::shared_ptr<T> resource)
	{
		if (key.empty() || resource == nullptr)
			return false;

		if (m_resources.find(key) != m_resources.end())
			return false;

		m_resources[key] = resource;
		return true;
	}


	template<typename T>
	std::vector<std::shared_ptr<T>> ResourceManager::LoadAll(const std::filesystem::path& path)
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
}
