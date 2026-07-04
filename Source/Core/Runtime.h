#pragma once
#include <memory>

namespace Dive
{
	class Window;
	class Graphics;
	class Renderer;

	class Runtime
	{
	public:
		static Runtime& GetInst()
		{
			static Runtime instance;
			return instance;
		}

		Runtime(const Runtime&) = delete;
		Runtime(Runtime&&) = delete;
		Runtime& operator=(const Runtime&) = delete;
		Runtime& operator=(Runtime&&) = delete;

		void Run();
		void Present();

	private:
		Runtime();
		~Runtime();

		void update();
		void render();

	private:
		Window* m_pWindow = nullptr;
		std::unique_ptr<Graphics> m_pGraphics = nullptr;
		std::unique_ptr<Renderer> m_pRenderer = nullptr;

		bool m_vSync = false;
	};
}