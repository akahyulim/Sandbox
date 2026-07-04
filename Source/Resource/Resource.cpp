#include "pch.h"
#include "Resource.h"

namespace Dive
{
	Resource::~Resource()
	{
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
