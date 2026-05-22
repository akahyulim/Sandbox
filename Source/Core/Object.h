#pragma once
#include <cstdint>
#include <string>

#include "Common.h"

namespace Dive
{
	constexpr uint64_t AUTO_ID = 0;

	class Object
	{
	public:
		explicit Object(uint64_t id = AUTO_ID)
		{
			m_instanceID = (id == AUTO_ID) ? GenerateUniqueID(1) : id;
		}
		virtual ~Object() = default;

		uint64_t GetInstanceID() const { return m_instanceID; }
		void SetInstanceID(uint64_t id) { m_instanceID = id; }

		virtual std::string GetName() const { return m_name; }
		virtual void SetName(const std::string& name) { m_name = name; }

		bool operator==(const Object& other) const { return m_instanceID == other.m_instanceID; }
		bool operator!=(const Object& other) const { return m_instanceID != other.m_instanceID; }

	private:
		uint64_t m_instanceID{};
		std::string m_name{};
	};

	inline bool operator==(const Object& lhs, const Object& rhs)
	{
		return lhs.GetInstanceID() == rhs.GetInstanceID();
	}

	inline bool operator!=(const Object& lhs, const Object& rhs)
	{
		return lhs.GetInstanceID() != rhs.GetInstanceID();
	}
}
