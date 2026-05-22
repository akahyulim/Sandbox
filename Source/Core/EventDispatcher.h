#pragma once
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <variant>

#define DV_EVENT_HANDLER(function)						[this](Dive::EventData data) { function();}
#define DV_EVENT_HANDLER_STATIC(function)				[](Dive::EventData data) { function();}

#define DV_EVENT_HANDLER_DATA(function)					[this](Dive::EventData data) { function(data);}
#define DV_EVENT_HANDLER_DATA_STATIC(function)			[](Dive::EventData data) { function(data);}

#define DV_SUBSCRIBE_EVENT(type, handler)				Dive::EventDispatcher::AddListener(type, this->GetInstanceID(), handler)
#define DV_UNSUBSCRIBE_EVENT(type)						Dive::EventDispatcher::RemoveListener(type, this->GetInstanceID())

#define DV_FIRE_EVENT(type)								Dive::EventDispatcher::Dispatch(type)
#define DV_FIRE_EVENT_DATA(type, data)					Dive::EventDispatcher::Dispatch(type, data)

namespace Dive
{
	enum class eEventType
	{
		WindowResized, Exit,

		Awake, Start,

		BeginFrame,

		FixedUpdate, Update, LateUpdate,

		PreRender, AfterOpaqueRender, AfterTransparentRender, PostRender,

		EndFrame,

		SceneCreated, SceneModified, SceneLoaded, SceneUnloaded,

		Collision, Trigger, KeyPress,
	};

	using EventData = std::variant<float, int, void*>;
	using EventListener = std::function<void(const EventData&)>;

	class EventDispatcher
	{
	public:
		static void AddListener(eEventType type, uint64_t instanceID, EventListener listener);
		static void RemoveListener(eEventType type, uint64_t instanceID);
		static void Dispatch(eEventType type, EventData data = 0);

	private:
		static std::unordered_map<eEventType, std::unordered_map<uint64_t, EventListener>> s_listeners;
	};
}