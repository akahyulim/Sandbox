#pragma once
#include <filesystem>
#include <string>
#include "Component.h"

namespace Dive
{
	class Script;

	class MonoBehaviour : public Component
	{
	public:
		MonoBehaviour(GameObject* owner, uint64_t id = AUTO_ID);
		~MonoBehaviour() override = default;

		bool IsEqualTo(const Component* other) const override { return false; };
		std::unique_ptr<Component> Clone(GameObject* owner) const override { return nullptr; };

		void Serialize(YAML::Emitter& out) override;
		void Deserialize(const YAML::Node& node) override;

		void Awake() override;
		void Start() override;

		void Update() override;

		void OnUpdate() override;
		void OnLateUpdate() override;

		void OnEnable() override;
		void OnDisable() override;
		void OnDestroy() override;

		//void OnCollisionEnter();
		//void OnCollisionStay();
		//void OnCollisionExit();

		//void OnTriggerEnter();
		//void OnTriggerStay();
		//void OnTriggerExit();

		//void OnPreRender();
		//void OnRenderObject();
		//void OnPostRender();

		std::weak_ptr<Script> GetScript() const { return m_script; }
		void SetScript(std::weak_ptr<Script> script) { m_script = script; }

		static constexpr eComponentType GetComponentType() { return eComponentType::MonoBehaviour; }

	private:
		std::weak_ptr<Script> m_script;
	};
}