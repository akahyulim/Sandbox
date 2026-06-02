#include "stdafx.h"
#include "Model.h"
//#include "Core/CoreSystem.h"
#include "StaticMesh.h"
#include "SkinnedMesh.h"
#include "Material.h"
#include "ResourceManager.h"
#include "API/Resources.h"

namespace Dive
{
	Model ::Model()
	{

	}

	Model::~Model()
	{

	}

	// Model.cpp
	bool Model::Create()
	{
		// Deserialize로 읽어온 m_nodes를 순회하며 실제 리소스를 로드함
		for (auto& node : m_nodes)
		{
			for (auto& inst : node.meshInstances)
			{
				// 경로 문자열을 이용해 리소스 매니저에서 가져옴
				inst.mesh = Resources::Load<StaticMesh>(inst.meshPath);	// 이 부분은 추후 변경해야 한다.
				inst.material = Resources::Load<Material>(inst.materialPath);
			}
		}
		return true;
	}

	void Model::Release()
	{

	}
	/*
	bool Model::SaveToFile(const std::filesystem::path& filepath)
	{
		std::filesystem::path target = filepath;
		target.replace_extension(".model");		// 추후 mdl로 수정?

		std::ofstream fout(target, std::ios::binary);
		if (!fout.is_open())
		{
			DV_LOG(Model, eLogLevel::Err, "[::SaveToFile] 파일 열기 실패: {}", target.string());
			return false;
		}

		if (!Serialize(fout))
			return false;

		ClearDirty();
		return true;
	}

	bool Model::LoadFromFile(const std::filesystem::path& filepath)
	{
		std::filesystem::path target = filepath;
		if (target.extension() != ".model")
		{
			target.replace_extension(".model");
		}

		std::ifstream fin(target, std::ios::binary);
		if (!fin.is_open())
		{
			DV_LOG(Model, eLogLevel::Err, "[::LoadFromFile] 파일 열기 실패: {}", target.string());
			return false;
		}

		if (!Deserialize(fin))
		{
			DV_LOG(Model, eLogLevel::Err, "[::LoadFromFile] 역직렬화 실패: {}", target.string());
			return false;
		}

		ClearDirty();
		return true;
	}
	*/
	bool Model::Serialize(std::ofstream& fout) const
	{
		if (!fout.is_open() || !fout.good()) return false;

		// 1. 모델 이름 저장
		std::string name = GetName();
		size_t nameLength = name.size();
		fout.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
		fout.write(name.c_str(), nameLength);

		// 2. 노드 수 저장
		size_t nodeCount = m_nodes.size();
		fout.write(reinterpret_cast<const char*>(&nodeCount), sizeof(nodeCount));

		for (const auto& node : m_nodes)
		{
			// 3. 노드 이름
			size_t nodeNameLength = node.name.size();
			fout.write(reinterpret_cast<const char*>(&nodeNameLength), sizeof(nodeNameLength));
			fout.write(node.name.c_str(), nodeNameLength);

			// 4. 부모 인덱스 및 로컬 행렬
			fout.write(reinterpret_cast<const char*>(&node.parentIndex), sizeof(int));
			fout.write(reinterpret_cast<const char*>(&node.localTransform), sizeof(DirectX::XMFLOAT4X4));

			// 5. 메시 인스턴스(Mesh + Material 경로) 저장
			size_t meshCount = node.meshInstances.size();
			fout.write(reinterpret_cast<const char*>(&meshCount), sizeof(meshCount));

			for (const auto& inst : node.meshInstances)
			{
				// Mesh 경로
				size_t meshPathLen = inst.meshPath.size();
				fout.write(reinterpret_cast<const char*>(&meshPathLen), sizeof(meshPathLen));
				fout.write(inst.meshPath.c_str(), meshPathLen);

				// Material 경로
				size_t matPathLen = inst.materialPath.size();
				fout.write(reinterpret_cast<const char*>(&matPathLen), sizeof(matPathLen));
				fout.write(inst.materialPath.c_str(), matPathLen);
			}
		}

		return true;
	}

	bool Model::Deserialize(std::ifstream& fin)
	{
		if (!fin.is_open() || !fin.good()) return false;

		m_nodes.clear();

		// 1. 모델 이름 읽기
		size_t nameLength = 0;
		fin.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
		std::string name;
		name.resize(nameLength);
		fin.read(&name[0], nameLength);
		SetName(name);

		// 2. 노드 수 읽기
		size_t nodeCount = 0;
		fin.read(reinterpret_cast<char*>(&nodeCount), sizeof(nodeCount));
		m_nodes.resize(nodeCount);

		for (uint32_t i = 0; i < nodeCount; ++i)
		{
			auto& node = m_nodes[i];

			// 3. 노드 이름 읽기
			size_t nodeNameLength = 0;
			fin.read(reinterpret_cast<char*>(&nodeNameLength), sizeof(nodeNameLength));
			node.name.resize(nodeNameLength);
			fin.read(&node.name[0], nodeNameLength);

			// 4. 부모 인덱스 및 로컬 행렬
			fin.read(reinterpret_cast<char*>(&node.parentIndex), sizeof(int));
			fin.read(reinterpret_cast<char*>(&node.localTransform), sizeof(DirectX::XMFLOAT4X4));

			// 5. 메시 인스턴스 정보 읽기
			size_t meshCount = 0;
			fin.read(reinterpret_cast<char*>(&meshCount), sizeof(meshCount));
			node.meshInstances.resize(meshCount);

			for (uint32_t j = 0; j < meshCount; ++j)
			{
				auto& inst = node.meshInstances[j];

				// Mesh 경로 읽기
				size_t meshPathLen = 0;
				fin.read(reinterpret_cast<char*>(&meshPathLen), sizeof(meshPathLen));
				std::string meshPathStr;
				meshPathStr.resize(meshPathLen);
				fin.read(&meshPathStr[0], meshPathLen);
				inst.meshPath = meshPathStr;

				// Material 경로 읽기
				size_t matPathLen = 0;
				fin.read(reinterpret_cast<char*>(&matPathLen), sizeof(matPathLen));
				std::string matPathStr;
				matPathStr.resize(matPathLen);
				fin.read(&matPathStr[0], matPathLen);
				inst.materialPath = matPathStr;
			}
		}

		return true;
	}
}