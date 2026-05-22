#pragma once
#include <cstdint>
#include <d3d11.h>

#include "Core/Types.h"

namespace Dive
{
	enum class eLoadOp
	{
		Clear,
		Load,
		OverWrite
	};

	struct RenderPass
	{
		uint32_t count = 0;

		ID3D11RenderTargetView* rtvs[8] = { nullptr };
		ID3D11DepthStencilView* dsv = nullptr;

		eLoadOp colorLoadOp = eLoadOp::Clear;
		eLoadOp depthLoadOp = eLoadOp::Clear;

		Color clearColor = Color::Black;
		float clearDepth = 1.0f;
		uint8_t clearStencil = 0;

		Viewport viewport;
	};
}