#include "pch.h"
#include "Resource.h"

namespace Dive
{
	Resource::~Resource()
	{
	}

	bool Resource::LoadFromFile(const std::filesystem::path& filePath)
	{
		if (!std::filesystem::exists(filePath) || !std::filesystem::is_regular_file(filePath))
		{
			spdlog::error("Resource::LoadFromFile - 파일이 없거나 올바르지 않습니다: {}", filePath.string());
			return false;
		}

		uintmax_t fileSize = std::filesystem::file_size(filePath);
		std::ifstream file(filePath, std::ios::binary | std::ios::ate);
		if (!file.is_open() || fileSize == 0)
		{
			spdlog::error("Resource::LoadFromFile - 파일을 열 수 없습니다: {}", filePath.string());
			return false;
		}

		std::vector<uint8_t> buffer;
		buffer.resize(static_cast<size_t>(fileSize));
		file.seekg(0, std::ios::beg);
		file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
		file.close();

		std::wstring extension = filePath.extension().wstring();

		if (!Deserialize(buffer, extension))
		{
			spdlog::error("Resource::LoadFromFile - Deserialize 실패: {}", filePath.string());
			return false;
		}

		spdlog::info("Resource::LoadFromFile - 로드 및 해독 완료: {}", filePath.string());
		return true;
	}

	void Resource::SetFilepath(const std::filesystem::path& path)
	{
		if (m_filepath != path)
		{
			m_filepath = path.generic_string();
		}
	}

	std::string Resource::GetName() const
	{
		const std::string& name = Object::GetName();
		if (!name.empty())	return name;

		if (m_filepath.empty()) return "Unknown_Resource";

		return m_filepath.stem().string();
	}
}
