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

	/*
	1. 자동 구독 해제 (Smart Handle): AddListener가 std::shared_ptr나 std::weak_ptr을 활용해 객체가 사라지면 자동으로 리스트에서 제거되도록 설계해 보세요.
	2. 데이터 구조화: std::variant도 훌륭하지만, Event라는 기본 클래스를 상속받는 구체적인 이벤트 클래스들을 만들어보세요. 
	그러면 Dispatch 시점에 데이터 해석이 훨씬 깔끔해집니다.
	*/
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