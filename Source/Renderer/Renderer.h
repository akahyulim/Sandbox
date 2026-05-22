#pragma once

namespace Dive
{
	class Graphics;
	struct RenderPass;

	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		bool Initialize(Graphics* graphics);

		void Render(const RenderPass* externalPass = nullptr);

	private:
	private:
		Graphics* m_graphics = nullptr;
	};
}