#include "stdafx.h"
#include "MonoBehaviour.h"
#include "../GameObject.h"
//#include "Core/CoreSystem.h"
#include "Resource/ResourceSystem.h"
#include "Script/ScriptSystem.h"

namespace Dive
{
	MonoBehaviour::MonoBehaviour(GameObject* owner, uint64_t id)
		: Component(owner, id)
	{
	}

	void MonoBehaviour::Serialize(YAML::Emitter& out)
	{
		if (!m_isDirty)
			return;

		out << YAML::Key << "MonoBehaviour" << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "ID" << YAML::Value << GetInstanceID();
		if(auto script = m_script.lock())
			out << YAML::Key << "Script" << YAML::Value << script->GetFilepath().string();
		out << YAML::EndMap;

		m_isDirty = false;
	}

	void MonoBehaviour::Deserialize(const YAML::Node& node)
	{
		if (GetInstanceID() != node["ID"].as<uint64_t>())
		{
			DV_LOG(MonoBehaviour, eLogLevel::Err, "[::Deserialize] InstanceID 초기화에 실패하였습니다.");
			return;
		}

		if(node["Script"])
			m_script = Resources::Load<Script>(node["Script"].as<std::string>());

		m_isDirty = true;
	}

	void MonoBehaviour::Awake()
	{
		if (auto script = m_script.lock())
			script->CallFunction("Awake");
	}

	void MonoBehaviour::Start()
	{
		if (auto script = m_script.lock())
			script->CallFunction("Start");
	}

	void MonoBehaviour::Update()
	{
		// 여기에선 뭘 해야할까?
	}
	
	void MonoBehaviour::OnUpdate()
	{
		// 잘 돌아가지만 일단 주석
		if (auto script = m_script.lock())
			script->CallFunction("OnUpdate");
	}
	
	void MonoBehaviour::OnLateUpdate()
	{
		if (auto script = m_script.lock())
			script->CallFunction("OnLateUpdate");
	}
	
	void MonoBehaviour::OnEnable()
	{
		if (auto script = m_script.lock())
			script->CallFunction("OnEnable");
	}
	
	void MonoBehaviour::OnDisable()
	{
		if (auto script = m_script.lock())
			script->CallFunction("OnDisable");
	}
	
	void MonoBehaviour::OnDestroy()
	{
		if (auto script = m_script.lock())
			script->CallFunction("OnDestroy");
	}
}