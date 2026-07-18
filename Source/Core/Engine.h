#pragma once
#include <memory>

namespace Dive
{
	class Window;
	class Timer;
	class Input;
	class Graphics;
	class Renderer;
	class Scene;

	struct EngineInit
	{
		bool vSync = false;
		Window* window = nullptr;
		std::string sceneFile;
	};

	class Engine
	{
	public:
		explicit Engine(const EngineInit& init);
		Engine(const Engine&) = delete;
		Engine(Engine&&) = delete;
		Engine& operator=(const Engine&) = delete;
		Engine& operator=(Engine&&) = delete;
		~Engine() = default;

		void Run();
		void Present();

		Window* GetWindow() const { return m_window; }
		Input* GetInput() const { return m_input.get(); }
		Graphics* GetGraphics() const { return m_graphics.get(); }

	private:
		void update();
		void render();

	private:
		Window* m_window = nullptr;
		std::unique_ptr<Timer> m_timer;
		std::unique_ptr<Input> m_input;
		std::unique_ptr<Graphics> m_graphics;
		std::unique_ptr<Renderer> m_renderer;
		std::unique_ptr<Scene> m_scene;

		bool m_vSync = false;
	};
}