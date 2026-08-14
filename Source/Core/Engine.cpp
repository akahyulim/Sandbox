#include "pch.h"
#include "Engine.h"
#include "Utilities/Timer.h"
#include "Graphics/Graphics.h"
#include "Rendering/TextureManager.h"	// ShaderManager랑 위치 맞추기
#include "Rendering/ShaderManager.h"
#include "Rendering/Renderer.h"
#include "Input/Input.h"
#include "Scene/Scene.h"

namespace Dive
{
	Engine::Engine(const EngineInit& init)
		: m_window(init.window),
		m_vSync(init.vSync)
	{
		m_input = std::make_unique<Input>(m_window->GetWindowHandle());

		m_graphics = std::make_unique<Graphics>(m_window);
		TextureManager::GetInst().Initialize(m_graphics.get());
		ShaderManager::GetInst().Initialize(m_graphics.get());
		m_renderer = std::make_unique<Renderer>(m_graphics.get(), m_window->GetWidth(), m_window->GetHeight());
		// create model importer

		m_window->GetResizedEvent().AddMember(&Graphics::ResizeBackbuffer, *m_graphics);
		m_window->GetResizedEvent().AddMember(&Renderer::OnResize, *m_renderer);

		m_timer = std::make_unique<Timer>();
		m_timer->Start();
	}

	// settings를 받는다.
	void Engine::Run()
	{
		m_timer->Tick();
		float dt = m_timer->GetDeltaTimeMS();

		if (m_input)
			m_input->Update();

		if(m_window->IsActive())
		{
			update(dt);
			render();
		}
	}
	
	void Engine::Present()
	{
		m_graphics->SwapBuffers(m_vSync);
	}
	
	Scene* Engine::NewScene()
	{
		m_scene.reset();
		m_scene = std::make_unique<Scene>();
		return m_scene.get();
	}

	// Renderer가 Tick, Update 그리고 Renderer로 나누어 호출된다.
	void Engine::update(float dt)
	{
		if (m_scene)
			m_scene->Update(dt);
		
		m_renderer->Update(dt);
	}
	
	void Engine::render()
	{
		m_renderer->Render();

		// 일단 외부 출력 기본설정
		m_renderer->ResolveToOffScreenTexture();

		// 1. Renderer::ResolveToOffScreenTexture()
		// 에디터모드같은 외부출력 설정이라면 위의 결과를 OffScreenRenderTarget에 최종 출력
		// 그리고 에디터에선 Renderer::GetOffScreenTexture()를 가져와서 ImGui::Image로 출력
		// 2. Renderer::ResolveToBackbuffer()
		// 엔진만 돌아간다면 백퍼에 결과물을 출력
	}
}