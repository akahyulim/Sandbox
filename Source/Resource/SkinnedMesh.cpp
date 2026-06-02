#include "stdafx.h"
#include "SkinnedMesh.h"

namespace Dive
{
	bool SkinnedMesh::Create()
	{
		DV_LOG(SkinnedMesh, eLogLevel::Err, "[::Create] 미구현");
		return false;
	}

	void SkinnedMesh::Release()
	{
		DV_LOG(SkinnedMesh, eLogLevel::Warn, "[::Release] 미구현");
	}

	bool SkinnedMesh::Create(GraphicsDevice* device, std::shared_ptr<IResourceDesc> desc)
	{
		DV_LOG(SkinnedMesh, eLogLevel::Err, "[::Create] 미구현");
		return false;
	}

	bool SkinnedMesh::Serialize(std::ofstream& fout) const
	{
		DV_LOG(SkinnedMesh, eLogLevel::Err, "[::Serialize] 미구현");
		return false;
	}

	bool SkinnedMesh::Deserialize(std::ifstream& fin)
	{
		DV_LOG(SkinnedMesh, eLogLevel::Err, "[::Deserialize] 미구현");
		return false;
	}
}