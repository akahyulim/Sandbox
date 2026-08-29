#pragma once
#include <string>
#include <unordered_map>
#include <memory>

#include "Mesh.h"

namespace Dive
{
	class Graphics;

	class MeshManager
	{
	public:
		static MeshManager& Get()
		{
			static MeshManager instance;
			return instance;
		}

		MeshManager(const MeshManager&) = delete;
		MeshManager& operator=(const MeshManager&) = delete;

		void Initialize(Graphics* graphics);
		void Shutdown();

		Mesh* GetMesh(const std::string& name);

	private:
		MeshManager() = default;
		~MeshManager() = default;

	private:
		Graphics* m_graphics = nullptr;
		std::unordered_map<std::string, std::unique_ptr<Mesh>> m_meshes;
	};
}