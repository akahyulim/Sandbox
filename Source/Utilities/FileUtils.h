#pragma once
#include <string>
#include <Windows.h>

namespace Dive
{
	namespace FileUtils
	{
		bool FileExists(const std::string& filepath);
		std::string GetFilename(const std::string filepath);
		std::string GetExtension(const std::string& filepath);

		std::filesystem::path OpenFile(const char* filter, HWND owner, const std::string& initialDir = "");
		std::filesystem::path SaveFile(const char* filter, HWND owner = NULL, const std::string& defaultPath = "");

		std::string SelectFolder(HWND owner = nullptr);
	};
}
