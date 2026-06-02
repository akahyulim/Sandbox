#include "stdafx.h"
#include "ResourceSerializer.h"
#include "ResourceDesc.h"
#include "StaticMesh.h"
#include "Material.h"

namespace Dive
{
	std::unique_ptr<IResourceData> MaterialImporter::Import(const std::filesystem::path& path)
	{
		return std::make_unique<MaterialData>();
	}


	// ===========================================================================================================================

	std::shared_ptr<IResourceDesc> StaticMeshSerializer::Load(const std::filesystem::path& path)
	{
		// 역시 이 부분은 중복된다.
		std::filesystem::path target = path;
		target.replace_extension(".dive");

		std::ifstream fin(target, std::ios::binary);
		if (!fin.is_open())
		{
			DV_LOG(StaticMeshSerializer, eLogLevel::Err, "파일 열기 실패: {}", target.string());
			return nullptr;
		}

		ResourceHeader header{};
		fin.read(reinterpret_cast<char*>(&header), sizeof(header));

		if (header.signature != 'EVID')
		{
			DV_LOG(StaticMeshSerializer, eLogLevel::Err, "파일 시그니쳐 불일치");
			return nullptr;
		}

		if (header.type != static_cast<uint32_t>(eResourceType::StaticMesh))
		{
			DV_LOG(StaticMeshSerializer, eLogLevel::Err, "파일 타입 불일치");
			return nullptr;
		}

		if (header.version > 1)
		{
			DV_LOG(StaticMeshSerializer, eLogLevel::Err, "파일 버전 불일치");
			return nullptr;
		}

		auto desc = std::make_shared<StaticMeshDesc>();

		// 🔹 헤더 읽기
		char magic[4];
		fin.read(magic, sizeof(magic));
		if (!fin || std::memcmp(magic, "MESH", 4) != 0)
		{
			DV_LOG(StaticMesh, eLogLevel::Err, "[::Deserialize] Magic 헤더 불일치");
			return nullptr;
		}

		size_t nameLength = 0;
		fin.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
		if (!fin || nameLength == 0 || nameLength > 1024)
		{
			DV_LOG(StaticMesh, eLogLevel::Err, "[::Deserialize] 이름 길이 오류");
			return nullptr;
		}

		std::string name(nameLength, '\0');
		fin.read(&name[0], nameLength);
		if (!fin)
		{
			DV_LOG(StaticMesh, eLogLevel::Err, "[::Deserialize] 이름 읽기 실패");
			return nullptr;
		}
		desc->name = name;

		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;
		fin.read(reinterpret_cast<char*>(&vertexCount), sizeof(vertexCount));
		fin.read(reinterpret_cast<char*>(&indexCount), sizeof(indexCount));
		if (!fin || vertexCount > 1'000'000 || indexCount > 3'000'000)
		{
			DV_LOG(StaticMesh, eLogLevel::Err, "[::Deserialize] 버텍스/인덱스 수치 오류");
			return nullptr;
		}

		desc->vertices.resize(vertexCount);
		fin.read(reinterpret_cast<char*>(desc->vertices.data()), sizeof(StaticVertex) * vertexCount);

		desc->indices.resize(indexCount);
		fin.read(reinterpret_cast<char*>(desc->indices.data()), sizeof(uint32_t) * indexCount);

		return desc;
	}

	std::shared_ptr<IResourceDesc> MaterialSerializer::Load(const std::filesystem::path& path)
	{
		std::filesystem::path target = path;
		target.replace_extension(".dive");

		std::ifstream fin(target, std::ios::binary);
		if (!fin.is_open())
		{
			DV_LOG(MaterialSerializer, eLogLevel::Err, "파일 열기 실패: {}", target.string());
			return nullptr;
		}

		ResourceHeader header{};
		fin.read(reinterpret_cast<char*>(&header), sizeof(header));

		if (header.signature != 'EVID')
		{
			DV_LOG(MaterialSerializer, eLogLevel::Err, "파일 시그니쳐 불일치");
			return nullptr;
		}

		if (header.type != static_cast<uint32_t>(eResourceType::Material))
		{
			DV_LOG(MaterialSerializer, eLogLevel::Err, "파일 타입 불일치");
			return nullptr;
		}

		if (header.version > 1)
		{
			DV_LOG(MaterialSerializer, eLogLevel::Err, "파일 버전 불일치");
			return nullptr;
		}

		// 이후 데이터를 파싱하고 객체를 만들어 리턴
		YAML::Node data;
		try
		{
			data = YAML::Load(fin);
		}
		catch (const YAML::Exception& e)
		{
			DV_LOG(MaterialSerializer, eLogLevel::Err, "YAML 처리 실패: {}", e.what());
			return nullptr;
		}

		auto desc = std::make_shared<MaterialDesc>();

		// 1. 수치 데이터 복구
		if (auto props = data["Properties"]) 
		{
			desc->params.diffuseColor = props["DiffuseColor"].as<DirectX::XMFLOAT4>();
			desc->params.tiling = props["Tiling"].as<DirectX::XMFLOAT2>();
			desc->params.offset = props["Offset"].as<DirectX::XMFLOAT2>();
			desc->params.flags = props["Flags"].as<uint32_t>();
		}

		// 2. 경로 문자열만 보관 (실제 로드는 하지 않음)
		if (auto maps = data["Maps"]) 
		{
			desc->mapPaths[static_cast<size_t>(eMapType::Diffuse)] = maps["Diffuse"].as<std::string>();
			desc->mapPaths[static_cast<size_t>(eMapType::Normal)] = maps["Normal"].as<std::string>();
		}

		// 3. 셰이더 이름 보관
		if (data["Shader"]) 
			desc->shaderName = data["Shader"].as<std::string>();

		return desc;
	}
}