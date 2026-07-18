#include "Core/Window.h"
#include "Core/Engine.h"
#include "Sandbox/Sandbox.h"

int main()
{
	// 파서부터 실행

	Dive::WindowInit window_init{};
	window_init.title = L"Sandbox";
	window_init.width = 1080;
	window_init.height = 720;
	window_init.maximize = FALSE;
	Dive::Window window(window_init);

	Dive::EngineInit engine_init{};
	engine_init.window = &window;
	engine_init.vSync = false;

	Dive::SandboxInit sandbox_init{};
	sandbox_init.engin_init = std::move(engine_init);
	Dive::Sandbox sandbox(sandbox_init);

	window.GetWindowEvent().Add([&](const Dive::WindowEventData& data) { sandbox.OnWindowEvent(data); });

	while (window.Run())
	{
		sandbox.Run();
	}

	return 0;
}