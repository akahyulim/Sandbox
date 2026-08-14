#pragma once
#include "Component.h"

#include <memory>

namespace Dive
{
	//class Material;
	//class StaticMesh;
	class GameObject;

	class MeshRenderer : public Component
	{
	public:
		MeshRenderer(GameObject* owner);
		virtual ~MeshRenderer() override = default;

		//std::shared_ptr<Material> GetMaterial() const { return m_material; }
		//void SetMaterial(std::shared_ptr<Material> mat) { m_material = mat; }

		//std::shared_ptr<StaticMesh> GetMesh() const { return m_mesh; }
		//void SetMesh(std::shared_ptr<StaticMesh> mesh) { m_mesh = mesh; }

		static constexpr eComponentType GetType() { return eComponentType::MeshRenderer; }

	private:
		//std::shared_ptr<Material> m_material;
		//std::shared_ptr<StaticMesh> m_mesh;
	};
}