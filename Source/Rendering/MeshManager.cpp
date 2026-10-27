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
			auto rawPtr = triangle.get();

			triangle->SetName("Triangle");
			m_meshes["Triangle"] = std::move(triangle);
			m_meshList.push_back(rawPtr);
		}
		// Preset Quad
		{
			data = Preset::GenerateQuad();
			auto quad = std::make_unique<Mesh>(graphics, data.vertices, data.indices);
			auto rawPtr = quad.get();

			quad->SetName("Quad");
			m_meshes["Quad"] = std::move(quad);
			m_meshList.push_back(rawPtr);
		}
		// Preset Plane
		{
			data = Preset::GeneratePlane();
			auto plane = std::make_unique<Mesh>(graphics, data.vertices, data.indices);
			auto rawPtr = plane.get();

			plane->SetName("Plane");
			m_meshes["Plane"] = std::move(plane);
			m_meshList.push_back(rawPtr);
		}
		// Preset Cube
		{
			data = Preset::GenerateCube();
			auto cube = std::make_unique<Mesh>(graphics, data.vertices, data.indices);
			auto rawPtr = cube.get();

			cube->SetName("Cube");
			m_meshes["Cube"] = std::move(cube);
			m_meshList.push_back(rawPtr);
		}
		// Preset Sphere
		{
			data = Preset::GenerateSphere();
			auto sphere = std::make_unique<Mesh>(graphics, data.vertices, data.indices);
			auto rawPtr = sphere.get();

			sphere->SetName("Sphere");
			m_meshes["Sphere"] = std::move(sphere);
			m_meshList.push_back(rawPtr);
		}
		// Preset Capsule
		{
			data = Preset::GenerateCapsule();
			auto capsule = std::make_unique<Mesh>(graphics, data.vertices, data.indices);
			auto rawPtr = capsule.get();

			capsule->SetName("Capsule");
			m_meshes["Capsule"] = std::move(capsule);
			m_meshList.push_back(rawPtr);
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