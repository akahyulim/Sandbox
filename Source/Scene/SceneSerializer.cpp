#include "pch.h"
#include "SceneSerializer.h"
#include "Scene.h"
#include "GameObject.h"
#include "Components/Transform.h"
#include "Components/Camera.h"
#include "Components/MeshRenderer.h"
#include "Components/Light.h"
//#include "Core/CoreSystem.h"
//#include "Rendering/RenderingSystem.h"
//#include "Resource/ResourceSystem.h"

using namespace DirectX;

namespace Dive
{
	SceneSerializer::SceneSerializer(Scene* scene)
		: m_scene(scene)
	{
		assert(scene);
	}

	// filepath보단 dir이 낫지 않을까?
	void SceneSerializer::Serialize(const std::filesystem::path& dir)
	{
		//if (!m_scene || !m_scene->IsDirty())
		//	return;

		// 여기에서 경로만 받아 확인 후 생성까지 하고
		// 이후 파일까지 생성하는 게 낫다.
		if (!std::filesystem::exists(dir))
		{
			std::filesystem::create_directories(dir);
		}

		std::filesystem::path filepath = dir / m_scene->GetName();
		filepath.replace_extension("scene");

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << m_scene->GetName();

		// Environment, MainCamera 등도 여기서 처리하는 게 맞다.

		out << YAML::Key << "GameObjects" << YAML::Value << YAML::BeginSeq;

		auto gameObjects = m_scene->GetAllGameObjects();
		for (auto gameObject : gameObjects)
		{
			//auto* info = gameObject->GetModelInfo();

			//if (info && !info->isRoot && !info->isModified)
			//	continue;

			out << YAML::BeginMap;

			gameObject->Serialize(out);
			/*
			if (info)
			{
				out << YAML::Key << "ModelInfo" << YAML::Value << YAML::BeginMap;
				out << YAML::Key << "IsRoot" << YAML::Value << info->isRoot;
				out << YAML::Key << "NodeName" << YAML::Value << info->modelNodeName;

				if (info->isRoot)
				{
					out << YAML::Key << "Path" << YAML::Key << info->modelPath;
				}
				else
				{
					out << YAML::Key << "ParentModelID" << YAML::Value << info->modelRoot->GetInstanceID();
				}

				out << YAML::EndMap;
			}
			*/
			out << YAML::EndMap;
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();

		spdlog::info( "월드 직렬화 완료: {}", m_scene->GetName());
	}

	std::unordered_map<uint64_t, Transform*> transformMap;

	bool SceneSerializer::Deserialize(const std::filesystem::path& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (const YAML::ParserException& e)
		{
			spdlog::warn( "[::Deserialize] 파싱 실패: {}", e.what());
			return false;
		}

		if (data["Scene"])
			m_scene->SetName(data["Scene"].as<std::string>());

		const auto& nodes = data["GameObjects"];
		if (!nodes || !nodes.IsSequence())
			return false;

		// 객체 ID로 검색하기 위한 임시 맵
		std::unordered_map<uint64_t, GameObject*> gameObjectMap;

		// 1st Pass: 객체 생성 및 기본 데이터 복구
		for (const auto& node : nodes)
		{
			auto instanceID = node["ID"].as<uint64_t>();
			auto name = node["Name"].as<std::string>();
			GameObject* gameObject = nullptr;

			/*
			// Model 정보 확인
			if (node["ModelInfo"])
			{
				const auto& modelInfoNode = node["ModelInfo"];
				bool isRoot = modelInfoNode["IsRoot"].as<bool>();

				if (isRoot)
				{
					// [루트인 경우] Instantiate로 전체 구조 생성
					auto path = modelInfoNode["Path"].as<std::string>();
					auto model = ResourceManager::Load<Model>(path);
					gameObject = m_scene->Instantiate(model, instanceID);
				}
				else
				{
					// [자식(Override)인 경우] 이미 생성된 루트를 찾아 그 아래에서 노드 이름으로 식별
					uint64_t parentModelID = modelInfoNode["ParentModelID"].as<uint64_t>();
					std::string nodeName = modelInfoNode["NodeName"].as<std::string>();

					GameObject* rootObject = gameObjectMap[parentModelID];
					if (rootObject)
					{
						// 루트 아래에서 이름이 일치하는 자식 객체를 찾음
						gameObject = rootObject->FindChildByName(nodeName);
						if (gameObject)
						{
							// 씬 파일에 기록된 ID로 강제 동기화 (나중에 부모 찾기용)
							gameObject->SetInstanceID(instanceID);
						}
					}
				}
			}
			else*/
			{
				// [일반 객체] 기존 방식대로 생성
				gameObject = m_scene->CreateGameObject(name, instanceID);
			}

			if (gameObject)
			{
				gameObject->Deserialize(node); // Transform, Component 등 덮어쓰기
				gameObjectMap[instanceID] = gameObject;
			}
		}

		/*
		// 2nd Pass: 계층 구조(Transform Parent) 연결
		// 이건 수정헤야 한다. 현재 Hierarchy는 모두 GameObject로 옮겼다.
		// Hierarchy때문에 Component에서 id를 사용했다면 이 역시 제거 대상이다.
		for (const auto& node : nodes)
		{
			auto id = node["Transform"]["ID"].as<uint64_t>();
			if (transformMap.find(id) == transformMap.end()) continue;

			auto transform = transformMap[id];
			if (node["Transform"]["Parent"])
			{
				auto parentID = node["Transform"]["Parent"].as<uint64_t>();
				if (transformMap.find(parentID) != transformMap.end())
				{
					auto parent = transformMap[parentID];
					//transform->SetParent(parent);
				}
			}
		}
		*/
		return true;
	}
}