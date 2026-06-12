#pragma once
#include <memory>
#include <unordered_map>

#include "Core/Object.h"
#include "Components/Component.h"

namespace Dive
{
	class Transform;

	class GameObject : public Object
	{
	public:
		GameObject();
		virtual ~GameObject() override;

		void Update();

		template<typename T>T* AddComponent();

		template<typename T>T* GetComponent() const;
		Component* GetComponentByType(eComponentType type) const;

		Transform* GetTransform() const { return m_transform.get(); }

		bool IsActive() const { return m_active; }
		void SetActive(bool active) { m_active = active; }

	private:
		std::unique_ptr<Transform> m_transform;
		std::unordered_map<eComponentType, std::unique_ptr<Component>> m_components;

		bool m_active = true;
	};


	template<typename T>
	T* GameObject::AddComponent()
	{
		eComponentType type = T::GetType();

		auto it = m_components.find(type);
		if (it != m_components.end())
			return static_cast<T*>(it->second.get());

		auto component = std::make_unique<T>(this);
		T* componentPtr = component.get();
		m_components[type] = std::move(component);

		return componentPtr;
	}

	template<typename T>
	T* GameObject::GetComponent() const
	{
		return static_cast<T*>(GetComponentByType(T::GetType()));
	}
}