#include "pch.h"
#include "MeshManager.h"
#include "Preset.h"
#include "Graphics/Graphics.h"

namespace Dive
{
	void MeshManager::Initialize(Graphics* graphics)
	{
		m_graphics = graphics;

		StaticGeometryData data{};

		// Preset Triangle
		{
			data = Preset::GenerateTriangle();
			auto triangle = std::make_unique<Mesh>(graphics, data.vertices, data.indices);
			m_meshes["Triangle"] = std::move(triangle);
		}
		// Preset Quad
		{
			data = Preset::GenerateQuad();
			auto quad = std::make_unique<Mesh>(graphics, data.vertices, data.indices);
			m_meshes["Quad"] = std::move(quad);
		}
		// Preset Plane
		{
			data = Preset::GeneratePlane();
			auto plane = std::make_unique<Mesh>(graphics, data.vertices, data.indices);
			m_meshes["Plane"] = std::move(plane);
		}
		// Preset Cube
		{
			data = Preset::GenerateCube();
			auto cube = std::make_unique<Mesh>(graphics, data.vertices, data.indices);
			m_meshes["Cube"] = std::move(cube);
		}
		// Preset Sphere
		{
			data = Preset::GenerateSphere();
			auto sphere = std::make_unique<Mesh>(graphics, data.vertices, data.indices);
			m_meshes["Sphere"] = std::move(sphere);
		}
		// Preset Capsule
		{
			data = Preset::GenerateCapsule();
			auto capsule = std::make_unique<Mesh>(graphics, data.vertices, data.indices);
			m_meshes["Capsule"] = std::move(capsule);
		}
	}

	void MeshManager::Shutdown()
	{
		m_meshes.clear();
	}
	
	Mesh* MeshManager::GetMesh(const std::string& name)
	{
		auto it = m_meshes.find(name);
		if (it != m_meshes.end())
			return it->second.get();

		return nullptr;
	}
}