#include "pch.h"
#include "Mesh.h"
#include "Graphics/Graphics.h"

namespace Dive
{
	Mesh::~Mesh() = default;
	
	/*
	bool Mesh::SaveToFile(const std::filesystem::path& filepath)
	{
		std::filesystem::path target = filepath;
		target.replace_extension(".mesh");

		std::ofstream fout(target, std::ios::binary);
		if (!fout.is_open())
		{
			DV_LOG(Mesh, eLogLevel::Err, "[::SaveToFile] 파일 열기 실패: {}", target.string());
			return false;
		}

		if (!Serialize(fout))	
			return false;

		ClearDirty();
		return true;
	}

	bool Mesh::LoadFromFile(const std::filesystem::path& filepath)
	{
		std::filesystem::path target = filepath;
		if (target.extension() != ".mesh")
		{
			// 전달받은 이름에 확장자가 아닌 .xxx형태면 강제로 변경되는 문제가 있다.
			//target.replace_extension(".mesh");
			target += ".mesh";
		}

		std::ifstream fin(target, std::ios::binary);
		if (!fin.is_open())
		{
			DV_LOG(Mesh, eLogLevel::Err, "[::LoadFromFile] 파일 열기 실패: {}", target.string());
			return false;
		}

		if (!Deserialize(fin))
		{
			DV_LOG(Mesh, eLogLevel::Err, "[::LoadFromFile] 역직렬화 실패: {}", target.string());
			return false;
		}

		ClearDirty();
		return true;
	}
	*/
}