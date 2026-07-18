#include "pch.h"
#include "Engine.h"
#include "Window.h"
#include "Utilities/Timer.h"
#include "Graphics/Graphics.h"
#include "Renderer/Renderer.h"
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
			// update(dt)
			// render(settings)
		}
	}
	
	void Engine::Present()
	{
		m_graphics->SwapBuffers(m_vSync);
	}
	
	// Renderer가 Tick, Update 그리고 Renderer로 나누어 호출된다.
	void Engine::update()
	{
		// camera->Tick(dt)
		// renderer->SetSceneViewportData
		// renderer->Tick(camera)
		// renderer->Update(dt)
	}
	
	void Engine::render()
	{
		// renderer->Render(settings)
	}
}