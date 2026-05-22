#include "pch.h"
#include "StringUtils.h"

namespace Dive
{
	namespace StringUtils
	{
		std::string ToUpperCase(const std::string& input)
		{
			auto result = input;
			for (char& c : result)
			{
				if (c >= 'a' && c <= 'z')
				{
					c -= ('a' - 'A');
				}
			}
			return result;
		}

		std::string ToLowerCase(const std::string& input)
		{
			auto result = input;
			for (char& c : result)
			{
				if (c >= 'A' && c <= 'Z')
				{
					c += ('a' - 'A');
				}
			}
			return result;
		}

		std::string StringLeftTrim(const std::string& input, const char* trimChars)
		{
			auto result = input;
			result.erase(0, result.find_first_not_of(trimChars));
			return result;
		}

		std::string StringRightTrim(const std::string& input, const char* trimChars)
		{
			auto result = input;
			result.erase(result.find_last_not_of(trimChars) + 1);
			return result;
		}

		std::string StringTrim(const std::string& input, const char* trimChars)
		{
			return StringLeftTrim(StringRightTrim(input, trimChars), trimChars);
		}

		std::string RemoveTrailingSlash(const std::string& path)
		{
			if (path.empty())
				return {};

			auto result = path;

			if (result.back() == '/')
				result.resize(result.length() - 1);

			return result;
		}

		std::string StringReplace(
			const std::string& input,
			const std::string& target,
			const std::string& replacement
		)
		{
			if (input.empty())
				return {};

			auto result = input;
			size_t position = 0;

			while ((position = result.find(target)) != std::string::npos)
				result.replace(position, target.size(), replacement);

			return result;
		}

		std::wstring StringToWString(const std::string& input)
		{
			if (input.empty())
				return {};

			auto size = ::MultiByteToWideChar(
				CP_ACP,
				0,
				input.c_str(),
				static_cast<int32_t>(input.size()),
				nullptr,
				0);

			std::wstring result(size, L'\0');

			::MultiByteToWideChar(
				CP_ACP,
				0,
				input.c_str(),
				static_cast<int32_t>(input.size()),
				result.data(),
				static_cast<int32_t>(result.size()));

			return result;
		}

		std::string WStringToString(const std::wstring& input)
		{
			if (input.empty())
				return {};

			auto size = ::WideCharToMultiByte(
				CP_ACP,
				0,
				input.c_str(),
				static_cast<int32_t>(input.size()),
				nullptr,
				0,
				nullptr,
				nullptr);

			std::string result(size, '\0');

			::WideCharToMultiByte(
				CP_ACP,
				0,
				input.c_str(),
				static_cast<int32_t>(input.size()),
				result.data(),
				static_cast<int32_t>(result.size()),
				nullptr,
				nullptr);

			return result;
		}

		std::vector<std::string> StringSplit(const std::string& input, char separator)
		{
			std::vector<std::string> result;

			size_t previous = 0;
			size_t current = input.find(separator);

			if (current == std::string::npos)
				result.emplace_back(input);
			else
			{
				while (current != std::string::npos)
				{
					result.emplace_back(input.substr(previous, current - previous));
					previous = current + 1;
					current = input.find(separator, previous);
				}
				result.emplace_back(input.substr(previous));
			}

			return result;
		}

		std::string SanitizeName(std::string name)
		{
			for (char& c : name)
			{
				if (c == '.' || c == ' ' || c == '\t')
					c = '_';
			}

			const std::string invalidChars = "\\/:*?\"<>|";
			name.erase(std::remove_if(name.begin(), name.end(), [&](char c) {
				return invalidChars.find(c) != std::string::npos;
				}), name.end());

			return name;
		}
	}
}