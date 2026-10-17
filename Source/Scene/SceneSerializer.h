#pragma once
#include <filesystem>

namespace Dive
{
	class Scene;

	class SceneSerializer
	{
	public:
		SceneSerializer(Scene* scene);
		~SceneSerializer() = default;

		void Serialize(const std::filesystem::path& dir = "Assets/Scenes");
		bool Deserialize(const std::filesystem::path& filepath);

	private:
		Scene* m_scene = nullptr;
	};
}
