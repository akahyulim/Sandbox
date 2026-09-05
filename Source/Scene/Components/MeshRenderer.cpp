#include "pch.h"
#include "MeshRenderer.h"
#include "Rendering/Mesh.h"
#include "Rendering/Material.h"
#include "Rendering/MaterialManager.h"
#include "Graphics/Graphics.h"

namespace Dive
{
	MeshRenderer::MeshRenderer(GameObject* owner)
		: Component(owner)
		, m_ObjectID(s_nextID++)
	{
		m_material = MaterialManager::Get().GetMaterial("Default");
	}

	void MeshRenderer::SetMesh(Mesh* mesh)
	{
		if (mesh && mesh->IsSkinned())
		{
			spdlog::error("스킨드 메시를 전달받았습니다.");
			return;
		}

		m_mesh = mesh;
	}

	void MeshRenderer::Draw(Graphics* graphics)
	{
		if (m_mesh)
		{
			graphics->SetTopology(m_topology);

			if (m_material)
				m_material->Bind(graphics);

			m_mesh->Bind(graphics);

			graphics->DrawIndexed(m_mesh->GetIndexCount());
		}
	}
}