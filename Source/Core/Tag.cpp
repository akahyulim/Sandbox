#include "pch.h"
#include "Tag.h"

namespace Dive
{
	Tag::Tag()
		: name("Untagged")
		, hash(HashString(name))
	{
	}

	Tag::Tag(const std::string& str)
		: name(str)
		, hash(HashString(name))
	{
	}
	
	Tag::Tag(const char* str)
		: name(str ? str : "Untagged")
		, hash(HashString(name))
	{
	}
	
	void Tag::Set(const std::string& str)
	{
		name = str;
		hash = HashString(str);
	}
}