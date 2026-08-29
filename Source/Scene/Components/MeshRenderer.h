#pragma once
#include "Component.h"

#include <memory>

namespace Dive
{
	class GameObject;
	class Material;
	class Mesh;
	class Graphics;

	class MeshRenderer : public Component
	{
	public:
		MeshRenderer(GameObject* owner);
		virtual ~MeshRenderer() override = default;

		Mesh* GetMesh() const { return m_mesh; }
		void SetMesh(Mesh* mesh);

		Material* GetMaterial() const { return m_material; }
		void SetMaterial(Material* material) { m_material = material; }

		void Draw(Graphics* graphics);

		static constexpr eComponentType GetType() { return eComponentType::MeshRenderer; }

	private:
		Mesh* m_mesh = nullptr;
		Material* m_material = nullptr;
	};
}