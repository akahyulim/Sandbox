#pragma once
#include <memory>

namespace Dive
{
	class Timer;
	class Graphics;
	class Renderer;
	class Scene;

	class Sandbox
	{
	public:
		Sandbox();
		~Sandbox();

		bool Initialize();
		void Run();

	private:
	private:
		std::unique_ptr<Timer> m_timer;
		std::unique_ptr<Graphics> m_graphics;
		std::unique_ptr<Renderer> m_renderer;

		std::unique_ptr<Scene> m_scene;
	};
}