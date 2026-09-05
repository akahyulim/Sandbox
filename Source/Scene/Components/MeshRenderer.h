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

		uint32_t GetObjectID() const { return m_ObjectID; }

		ePrimitiveTopology GetTopology() const { return m_topology; }
		void SetTopology(ePrimitiveTopology topology) { m_topology = topology; }

		static constexpr eComponentType GetType() { return eComponentType::MeshRenderer; }

	private:
		uint32_t m_ObjectID;
		static inline uint32_t s_nextID = 1;

		Mesh* m_mesh = nullptr;
		Material* m_material = nullptr;

		ePrimitiveTopology m_topology = ePrimitiveTopology::TriangleList;
	};
}