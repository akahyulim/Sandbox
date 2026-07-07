#pragma once
#include <memory>
#include <unordered_map>

#include "Core/Object.h"
#include "Components/Component.h"

namespace Dive
{
	class Scene;
	class Transform;

	class GameObject : public Object
	{
	public:
		GameObject(uint64_t id);
		explicit GameObject(uint64_t id, Scene* scene);
		GameObject(const GameObject&) = delete;
		virtual ~GameObject() override;

		GameObject& operator=(const GameObject&) = delete;

		void Update(float dt);

		template<typename T>T* AddComponent();

		template<typename T>T* GetComponent() const;
		Component* GetComponentByType(eComponentType type) const;

		Transform* GetTransform() const { return m_transform.get(); }

		bool IsActive() const { return m_active; }
		void SetActive(bool active) { m_active = active; }

		// Hierarchy
		bool HasParent() const { return m_parent != nullptr; }
		GameObject* GetParent() const { return m_parent; }
		void SetParent(GameObject* parent);
		void DetachFromParent();

		uint32_t GetChildCount() { return static_cast<uint32_t>(m_children.size()); }
		bool HasChildren() const { return !m_children.empty(); }
		const std::vector<std::unique_ptr<GameObject>>& GetChildren() { return m_children; }
		void AddChild(std::unique_ptr<GameObject> child);
		std::unique_ptr<GameObject> RemoveChild(GameObject* child);

	private:
		void notifySceneChanged();

	private:
		Scene* m_scene = nullptr;
		GameObject* m_parent = nullptr;
		std::vector<std::unique_ptr<GameObject>> m_children;

		std::unique_ptr<Transform> m_transform;
		std::unordered_map<eComponentType, std::unique_ptr<Component>> m_components;

		bool m_active = true;

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