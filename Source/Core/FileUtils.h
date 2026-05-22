#pragma once
#include <string>
#include <filesystem>
#include <Windows.h>

namespace Dive
{
	namespace FileUtils
	{
		std::filesystem::path OpenFile(const char* filter, HWND owner, const std::string& initialDir = "");
		std::filesystem::path SaveFile(const char* filter, HWND owner = NULL, const std::string& defaultPath = "");

		std::string SelectFolder(HWND owner = nullptr);
	};
}
