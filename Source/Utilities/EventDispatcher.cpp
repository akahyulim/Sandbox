#include "pch.h"
#include "EventDispatcher.h"

namespace Dive
{
	std::unordered_map<eEventType, std::unordered_map<uint64_t, EventListener>> EventDispatcher::s_listeners;

	void EventDispatcher::AddListener(eEventType type, uint64_t instanceID, EventListener listener)
	{
		s_listeners[type][instanceID] = listener;
	}

	void EventDispatcher::RemoveListener(eEventType type, uint64_t instanceID)
	{
		s_listeners[type].erase(instanceID);
	}

	void EventDispatcher::Dispatch(eEventType type, EventData data)
	{
		auto& typeListener = s_listeners[type];
		for (auto& [instanceID, listener] : typeListener)
			listener(data);
	}
}