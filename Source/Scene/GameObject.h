#pragma once
#include <memory>
#include <unordered_map>

#include "Core/Object.h"
#include "Components/Component.h"

namespace Dive
{
	class Scene;
	class Component;
	class Transform;

	class GameObject : public Object
	{
	public:
		GameObject(const std::string& name = "GameObject");
		GameObject(Scene* scene, uint64_t id = AUTO_ID, const std::string& name = "GameObject");
		GameObject(const GameObject&) = delete;
		virtual ~GameObject() override;

		GameObject& operator=(const GameObject&) = delete;

		void Update();

		template<typename T>T* AddComponent();

		template<typename T>T* GetComponent() const;
		Component* GetComponentByType(eComponentType type) const;

		Transform* GetTransform() const { return m_transform.get(); }

		bool IsActive() const;
		void SetActive(bool active) { m_isActive = active; }

		// Hierarchy
		GameObject* GetRoot() { return m_parent ? m_parent->GetRoot() : this; }
		bool HasParent() const { return m_parent != nullptr; }
		GameObject* GetParent() const { return m_parent; }
		void SetParent(GameObject* parent);
		void DetachFromParent();

		uint32_t GetChildCount() { return static_cast<uint32_t>(m_children.size()); }
		std::vector<GameObject*> GetChildren() { return m_children; }	// 일단 복사로 변경
		GameObject* GetChildByIndex(uint32_t index) const;
		bool HasChildren() const { return !m_children.empty(); }
		void DetachChildren();
		void RemoveChild(GameObject* child);
		bool IsDescendantOf(GameObject* target) const;
		void GetDecendants(std::vector<GameObject*>& outDecendants) const;

	private:
		void notifySceneChanged();

	private:
		Scene* m_scene = nullptr;
		GameObject* m_parent = nullptr;
		std::vector<GameObject*> m_children;

		std::unique_ptr<Transform> m_transform;
		std::unordered_map<eComponentType, std::unique_ptr<Component>> m_components;

		bool m_isActive = true;

		friend class Scene;
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

		notifySceneChanged();

		return componentPtr;
	}

	template<typename T>
	T* GameObject::GetComponent() const
	{
		return static_cast<T*>(GetComponentByType(T::GetType()));
	}
}