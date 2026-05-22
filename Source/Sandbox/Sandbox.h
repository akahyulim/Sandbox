#pragma once
#include <memory>

namespace Dive
{
	class Graphics;
	class Renderer;

	class Sandbox
	{
	public:
		Sandbox();
		~Sandbox();

		bool Initialize();
		void Run();

	private:
	private:
		std::unique_ptr<Graphics> m_graphics;
		std::unique_ptr<Renderer> m_renderer;
	};
}