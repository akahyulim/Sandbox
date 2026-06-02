#pragma once
#include <string>
#include <filesystem>
#include <fstream>

#include "Core/Object.h"

namespace Dive
{
	enum class eResourceType
	{
		Undefined,
		Texture2D,
		RenderTexture,
		Cubemap,
		StaticMesh,
		SkinnedMesh,
		Material,
	};

	struct ResourceHeader
	{
		uint32_t signature;
		uint32_t version;
		uint32_t type;
	};

	class Graphics;

	class Resource : public Object
	{
	public:
		Resource() = default;
		virtual ~Resource() override;

		bool LoadFromFile(const std::filesystem::path& filepath);
		virtual bool Deserialize(const std::vector<uint8_t>& fileBuffer, std::wstring_view extension) = 0;

		virtual bool Create(Graphics*) = 0;

		const std::filesystem::path& GetFilepath() const { return m_filepath; }
		void SetFilepath(const std::filesystem::path& path);

		virtual eResourceType GetType() const { return eResourceType::Undefined; }
		static constexpr eResourceType StaticType() { return eResourceType::Undefined; }

		std::string GetName() const override;

		bool IsDirty() const { return m_isDirty; }

	protected:
		void MarkDirty() { m_isDirty = true; }
		void ClearDirty() { m_isDirty = false; }

	protected:
		std::filesystem::path m_filepath{};
		bool m_isDirty = true;
	};
}
