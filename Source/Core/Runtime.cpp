#include "pch.h"
#include "Runtime.h"
#include "Graphics/Graphics.h"
#include "Renderer/Renderer.h"

namespace Dive
{
	Runtime::Runtime()
	{
	}

	Runtime::~Runtime()
	{
	}
	
	// settings를 받는다.
	void Runtime::Run()
	{
		// timer
		// dt

		// input.tick

		// window->IsActive
		// update(dt)
		// render(settings)
	}
	
	void Runtime::Present()
	{
		// graphics->SwapBuffers(vSync);
	}
	
	void Runtime::update()
	{
		// camera->Tick(dt)
		// renderer->SetSceneViewportData
		// renderer->Tick(camera)
		// renderer->Update(dt)
	}
	
	void Runtime::render()
	{
		// renderer->Render(settings)
	}
}