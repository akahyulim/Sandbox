#pragma once
#include <cstdint>
#include <optional>
#include <vector>
#include <d3d11.h>

namespace Dive
{
	enum class eLoadAccessOp : uint8_t
	{
		Load,
		Clear,
		DontCare
	};

	struct RenderTargetDesc
	{
		eLoadAccessOp AccessType = eLoadAccessOp::Clear;
		ID3D11RenderTargetView* RenderTargetView = nullptr;
		float ClearColor[4]{};
	};

	struct DepthStencilDesc
	{
		eLoadAccessOp AccessType = eLoadAccessOp::Clear;
		ID3D11DepthStencilView* DepthStencilView = nullptr;
		uint32_t ClearFlags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL; 
		float Depth = 1.0f;
		uint8_t Stencil = 0;
	};

	struct RenderPassDesc
	{
		std::vector<RenderTargetDesc> renderTargetDescs;
		std::optional<DepthStencilDesc> depthStencilDesc;
	};
}