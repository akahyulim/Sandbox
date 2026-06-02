#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <assert.h>
#include <functional>
#include "Core/Object.h"
#include "Components/Component.h"
//#include "Components/Transform.h"
//#include "Components/MonoBehaviour.h"

namespace Dive
{
	class Scene;
	class Prefab;

	class GameObject;

	struct ModelInstanceInfo
	{
		std::string modelPath;
		std::string modelNodeName;		// 수정된 노드 이름

		GameObject* modelRoot = nullptr;
		bool isRoot = false;
		bool isModified = false;
	};

	class GameObject : public Object
	{
	public:
		GameObject(const std::string& name = "GameObject", uint64_t id = AUTO_ID);
		GameObject(Scene* scene, const std::string& name = "GameObject", uint64_t id = AUTO_ID);
		~GameObject() override;

		void Update();

		std::unique_ptr<GameObject> Clone(Scene* scene, uint64_t id = AUTO_ID) const;

		//virtual void Serialize(YAML::Emitter& out);
		//virtual void Deserialize(const YAML::Node& node);

		bool IsDestroyed() const { return m_isDestroyed; }

		void SetTag(const std::string& tag) { m_tag = tag; }
		std::string GetTag() const { return m_tag; }
		bool CompareTag(const std::string& tag) const { return m_tag == tag; }

		void SetActive(bool value);
		bool IsActiveSelf() const { return m_isActiveSelf; }
		bool IsActiveHierarchy() const { return m_isActiveHierarchy; }

		bool IsModelInstance() const { return m_modelInfo != nullptr; }
		ModelInstanceInfo* GetModelInfo() { return m_modelInfo.get(); }
		void SetModelInfo(std::unique_ptr<ModelInstanceInfo> info) { m_modelInfo = std::move(info); }
		bool IsModelRoot() const { return IsModelInstance() && m_modelInfo->isRoot; }

		template<typename T> T* AddComponent(uint64_t id = AUTO_ID);

		template<typename T> void RemoveComponent();
		void RemoveComponentByType(eComponentType type);

		template<typename T> bool HasComponent() const;
		bool HasComponentByType(eComponentType type) const;

		template<typename T> T* GetComponent() const;
		Component* GetComponentByType(eComponentType type) const;

		template<typename T> std::vector<T*> GetComponentsInChildren(bool includeInactive = false) const;

		Scene* GetWorld() const { return m_scene; }
		void SetWorld(Scene* scene) { m_scene = scene; }

		void OnParentChanged();

		Transform* GetTransform() const;

		GameObject* GetParent() const;
		void SetParent(GameObject* parent);

		GameObject* FindChildByName(const std::string& name);
		/*
		const DirectX::XMFLOAT3& GetPosition() const { return m_transform->GetPosition(); }
		void SetPosition(const DirectX::XMFLOAT3& pos) { m_transform->SetPosition(pos); }
		const DirectX::XMFLOAT3& GetLocalPosition() const { return m_transform->GetLocalPosition(); }
		void SetLocalPosition(const DirectX::XMFLOAT3& pos) { m_transform->SetLocalPosition(pos); }

		const DirectX::XMFLOAT3& GetRotation() const { return m_transform->GetRotationDegrees(); }
		void SetRotation(const DirectX::XMFLOAT3& degrees) { m_transform->SetRotationByDegrees(degrees); }
		const DirectX::XMFLOAT3& GetLocalRotation() const { return m_transform->GetLocalRotationDegrees(); }
		void SetLocalRotation(const DirectX::XMFLOAT3& degrees) { m_transform->SetLocalRotationByDegrees(degrees); }

		const DirectX::XMFLOAT3& GetScale() const { return m_transform->GetScale(); }
		void SetScale(const DirectX::XMFLOAT3& scale) { m_transform->SetScale(scale); }
		const DirectX::XMFLOAT3& GetLocalScale() const { return m_transform->GetLocalScale(); }
		void SetLocalScale(const DirectX::XMFLOAT3& scale) { m_transform->SetLocalScale(scale); }

		void Translate(const DirectX::XMFLOAT3& move, eSpace space = eSpace::Local) { m_transform->Translate(move, space); }

		void Rotate(const DirectX::XMFLOAT3& deltaDegrees, eSpace space = eSpace::Local) { m_transform->RotateByDegrees(deltaDegrees, space); }

		void LookAt(const DirectX::XMFLOAT3 target) { m_transform->LookAt(target); }

		DirectX::XMFLOAT3 GetForward() const { return m_transform->GetForward(); }
		DirectX::XMFLOAT3 GetUp() const { return m_transform->GetUp(); }
		DirectX::XMFLOAT3 GetRight() const { return m_transform->GetRight(); }
		*/
	private:
		void updateActiveInHierarchy(bool parentHierarchy);

	private:
		std::unique_ptr<ModelInstanceInfo> m_modelInfo = nullptr;

		std::string m_tag = "Untagged";
		// layer
		
		bool m_isDestroyed = false;
		bool m_isActiveSelf = true;
		bool m_isActiveHierarchy = true;

		Scene* m_scene = nullptr;
		std::unique_ptr<Transform> m_transform;
		std::unordered_map<eComponentType, std::unique_ptr<Component>> m_components;
		//std::vector<std::unique_ptr<MonoBehaviour>> m_scripts;

		friend class Scene;
		friend class Prefab;
	};

	template<typename T>
	T* GameObject::AddComponent(uint64_t id)
	{
		if constexpr (std::is_same_v<T, Transform>)
		{
			return m_transform.get();
		}

		eComponentType type = T::GetComponentType();

		if (m_components.find(type) != m_components.end())
		{
			return static_cast<T*>(m_components[type].get());
		}

		auto component = std::make_unique<T>(this, id);
		T* componentPtr = component.get();
		m_components[type] = std::move(component);

		return componentPtr;
	}

	template<typename T>
	void GameObject::RemoveComponent()
	{
		static_assert(std::is_base_of<Component, T>::value, "T는 반드시 Component를 상속하여야 한다.");
		RemoveComponentByType(T::GetComponentType());
	}

	template<typename T>
	bool GameObject::HasComponent() const
	{
		static_assert(std::is_base_of<Component, T>::value, "T는 반드시 Component를 상속하여야 한다.");
		return HasComponentByType(T::GetComponentType());
	}

	template<typename T>
	T* GameObject::GetComponent() const
	{
		static_assert(std::is_base_of<Component, T>::value, "T는 반드시 Component를 상속하여야 한다.");
		return static_cast<T*>(GetComponentByType(T::GetComponentType()));
	}

	template<typename T>
	std::vector<T*> GameObject::GetComponentsInChildren(bool includeInactive) const
	{
		std::vector<T*> components;

		std::function<void(const GameObject*)> collect;

		collect = [&](const GameObject* obj) {
			if (obj->HasComponent<T>())
			{
				if (includeInactive || obj->m_isActiveSelf)
					components.push_back(obj->GetComponent<T>());
			}

			for (const Transform* child : obj->GetTransform()->GetChildren())
				collect(child->GetGameObject());
			};

		collect(this);

		return components;
	}

}
