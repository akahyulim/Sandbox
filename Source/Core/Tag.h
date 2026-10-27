#pragma once
#include <string>
#include <cstdint>

namespace Dive
{
	struct Tag
	{
		Tag();
		Tag(const std::string& str);
		Tag(const char* str);

		bool operator==(uint32_t otherHash) const { return hash == otherHash; }
		bool operator!=(uint32_t otherHash) const { return hash != otherHash; }

		bool operator==(const Tag& other) const { return hash == other.hash; }
		bool operator!=(const Tag& other) const { return hash != other.hash; }

		bool operator==(const std::string& otherName) const { return name == otherName; }
		bool operator!=(const std::string& otherName) const { return name != otherName; }

		static constexpr uint32_t HashString(const char* str)
		{
			uint32_t hash = 2166136261u;
			while (*str)
			{
				hash ^= static_cast<uint32_t>(*str++);
				hash *= 16777619u;
			}
			return hash;
		}

		static uint32_t HashString(const std::string& str)
		{
			return HashString(str.c_str());
		}

		void Set(const std::string& str);

		std::string name;
		uint32_t hash = 0;
	};

	constexpr uint32_t EditorOnly = Tag::HashString("EditorOnly");
	constexpr uint32_t MainCamera = Tag::HashString("MainCamera");
	constexpr uint32_t Player = Tag::HashString("Player");
	constexpr uint32_t Enemy = Tag::HashString("Enemy");
}