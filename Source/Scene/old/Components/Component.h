#pragma once
#include <memory>
#include <nlohmann/json.hpp>
#include "Core/Object.h"

namespace Dive
{
	class GameObject;
	class Transform;

	enum class eComponentType
	{
		Undefined,
		Transform,
		Camera,
		Light,
		MeshRenderer,
		SkinnedMeshRender,
		MonoBehaviour
	};

	class Component : public Object
	{
	public:
		Component(GameObject* owner, uint64_t id);
		~Component() override = default;

		void SubscribeEvents();
		void UnsubscribeEvents();

		virtual bool IsEqualTo(const Component* other) const = 0;
		virtual std::unique_ptr<Component> Clone(GameObject* owner) const = 0;

		//virtual void Serialize(YAML::Emitter& out) = 0;
		//virtual void Deserialize(const YAML::Node& node) = 0;

		virtual void Awake() {}
		virtual void Start() {}

		virtual void Update() {}

		virtual void OnUpdate() {}
		virtual void OnLateUpdate() {}

		virtual void OnEnable() {}
		virtual void OnDisable() {}
		virtual void OnDestroy() {}

		bool IsOverrides(const Component* prefabComponent) const;

		std::string GetName() const override;

		GameObject* GetGameObject() const;
		Transform* GetTransform() const;

		bool IsDirty() const { return m_isDirty; }

		static constexpr eComponentType GetComponentType() { return eComponentType::Undefined; }

	protected:
		GameObject* m_owner = nullptr;
		bool m_isDirty = true;
	};
}