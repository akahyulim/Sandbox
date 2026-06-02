#pragma once
#include <filesystem>
#include <memory>

namespace Dive
{
	class DvResource;
	class IResourceData;

	class IResourceImporter
	{
	public:
		virtual ~IResourceImporter() = default;

		virtual std::unique_ptr<IResourceData> Import(const std::filesystem::path& path) = 0;
	};

	class MaterialImporter : public IResourceImporter
	{
	public:
		std::unique_ptr<IResourceData> Import(const std::filesystem::path& path);
	};

	// ======================================================================================================

	class Resource;
	class IResourceDesc;

	// Loader를 Serializer로 바꾸고 직렬화를 추가하자.
	// 이때 구체 리소스는 GetDesc를 제공해야하며 이를 이용해 직렬화를 수행하도록 해야한다.
	class IResourceSerializer
	{
	public:
		virtual ~IResourceSerializer() = default;

		virtual bool Save(const std::filesystem::path& path, std::shared_ptr<IResourceDesc> desc) { return false; }
		virtual std::shared_ptr<IResourceDesc> Load(const std::filesystem::path& path) = 0;
	};

	class StaticMeshSerializer : public IResourceSerializer
	{
	public:
		StaticMeshSerializer() = default;
		~StaticMeshSerializer() override = default;

		std::shared_ptr<IResourceDesc> Load(const std::filesystem::path& path) override;
	};

	class MaterialSerializer : public IResourceSerializer
	{
	public:
		MaterialSerializer() = default;
		~MaterialSerializer() override = default;

		std::shared_ptr<IResourceDesc> Load(const std::filesystem::path& path) override;
	};
}