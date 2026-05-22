#include "pch.h"
#include "Component.h"
#include "../GameObject.h"
#include "Core/EventDispatcher.h"

namespace Dive
{
	Component::Component(GameObject* owner, uint64_t id)
		: Object(id)
		, m_owner(owner)
	{
	}

	// Update는 계층구조 유지 및 갱신을 위해 GameObject가 하향식으로 호출하는 게 맞다.
	// 따라서 이벤트 콜백 메서드는 OnXXX 형태로 추가하는 게 나을 것 같다.
	void Component::SubscribeEvents()
	{
		DV_SUBSCRIBE_EVENT(eEventType::Update, DV_EVENT_HANDLER(OnUpdate));
		DV_SUBSCRIBE_EVENT(eEventType::LateUpdate, DV_EVENT_HANDLER(OnLateUpdate));
		//DV_SUBSCRIBE_EVENT(eEventType::Update, DV_EVENT_HANDLER(OnEnable));
		//DV_SUBSCRIBE_EVENT(eEventType::Update, DV_EVENT_HANDLER(OnDisable));
		//DV_SUBSCRIBE_EVENT(eEventType::Update, DV_EVENT_HANDLER(OnDestroy));
	}

	void Component::UnsubscribeEvents()
	{
		DV_UNSUBSCRIBE_EVENT(eEventType::Update);
		DV_UNSUBSCRIBE_EVENT(eEventType::LateUpdate);
	}

	bool Component::IsOverrides(const Component* prefabComponent) const
	{
		return !IsEqualTo(prefabComponent);
	}

	std::string Component::GetName() const
	{
		return m_owner != nullptr ? m_owner->GetName() : std::string();
	}

	GameObject* Component::GetGameObject() const
	{
		return m_owner;
	}

	Transform* Component::GetTransform() const
	{
		return m_owner != nullptr ? m_owner->GetTransform() : nullptr;
	}
}
