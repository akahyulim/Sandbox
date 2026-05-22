#include "pch.h"
#include "Sandbox.h"

int main()
{
	spdlog::info("샌드박스 시작");

	Dive::Sandbox sandbox;
	if (sandbox.Initialize())
		sandbox.Run();

	spdlog::info("샌드박스 종료");

	return 0;
}