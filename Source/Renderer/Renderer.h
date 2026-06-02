#pragma once

namespace Dive
{
	class Graphics;
	struct RenderPass;
	class Scene;

	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		bool Initialize(Graphics* graphics);

		void Render(Scene* scene);

	private:
	private:
		Graphics* m_graphics = nullptr;
	};
}