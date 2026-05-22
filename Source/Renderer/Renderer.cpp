#include "pch.h"
#include "Renderer.h"
#include "Graphics/Graphics.h"
#include "Graphics/RenderPass.h"
#include "Graphics/PipelineState.h"

namespace Dive
{
	Renderer::Renderer()
	{
	}
	
	Renderer::~Renderer()
	{
	}

	bool Renderer::Initialize(Graphics* graphics)
	{
		assert(graphics != nullptr);
		m_graphics = graphics;
		// 초기화 완료 확인
		return true;
	}

	void Renderer::Render(const RenderPass* externalPass)
	{
		// externalPass는 추후 적용

		// 테스트용 간단한 패스
		RenderPass pass;
		pass.clearColor = Color::DeepSkyBlue;
		pass.count = 1;
		pass.rtvs[0] = m_graphics->GetRenderTargetView();
		pass.dsv = m_graphics->GetDepthStencilView();

		m_graphics->BeginRenderPass(pass);
		// pipeline은 일단 제외
		// -> Drawable들로 채워진다.
		m_graphics->EndRenderPass();
	}
}