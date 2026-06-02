#pragma once

namespace Dive
{
	enum class eComponentType
	{
		Undefined,
		Transform,
		Camera,
		Light,
		MeshRenderer,
	};

	class Transform;
	class GameObject;

	class Component
	{
	public:
		Component(GameObject* gameObject);
		virtual ~Component() = default;
		
		virtual void Awake() {}
		virtual void Start() {}

		virtual void Update() {}

		virtual void OnUpdate() {}
		virtual void OnLateUpdate() {}

		virtual void OnEnable() {}
		virtual void OnDisable() {}
		virtual void OnDestroy() {}

		//std::string GetName() const override;

		GameObject* GetGameObject() const;
		Transform* GetTransform() const;

		bool IsDirty() const { return m_isDirty; }

		static constexpr eComponentType GetType() { return eComponentType::Undefined; }

	protected:
		GameObject* m_owner = nullptr;
		bool m_isDirty = true;
	};
}