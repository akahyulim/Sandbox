#pragma once
#include "Mesh.h"

namespace Dive
{
	class StaticMesh : public Mesh
	{
	public:
		StaticMesh(): Mesh(eMeshType::Static) {}
		virtual ~StaticMesh() override;

		//eResourceType GetType() const override { return eResourceType::StaticMesh; }
		//static constexpr eResourceType StaticType() { return eResourceType::StaticMesh; }

	private:
	};
}
