#pragma once
#include <string>
#include <vector>

namespace Dive
{
	namespace StringUtils
	{
		std::string ToUpperCase(const std::string& input);
		std::string ToLowerCase(const std::string& input);
		std::string StringLeftTrim(const std::string& input, const char* trimChars = " \t\n\r\f\v");
		std::string StringRightTrim(const std::string& input, const char* trimChars = " \t\n\r\f\v");
		std::string StringTrim(const std::string& input, const char* trimChars = " \t\n\r\f\v");
		std::string RemoveTrailingSlash(const std::string& input);
		std::string StringReplace(const std::string& input, const std::string& target, const std::string& replacement);
		std::wstring StringToWString(const std::string& input);
		std::string WStringToString(const std::wstring& input);
		std::vector<std::string> StringSplit(const std::string& input, char separator);

		std::string SanitizeName(std::string name);
	};
}
