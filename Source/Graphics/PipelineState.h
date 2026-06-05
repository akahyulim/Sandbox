#pragma once
#include <cstdint>
#include <memory>

#include "Core/Types.h"

namespace Dive
{
	class ShaderProgram;

	struct PipelineState
	{
		ePrimitiveTopology topology = ePrimitiveTopology::None;
		
		//std::shared_ptr<VertexShader> vertexShader;
		//std::shared_ptr<PixelShader> pixelShader;
		//std::shared_ptr<InputLayout> inputLayout;
		std::shared_ptr<ShaderProgram> shaderProgram;
		
		eDepthStencilState depthStencilState = eDepthStencilState::Count;
		eRasterizerState rasterizerState = eRasterizerState::Count;
		eBlendState blendState = eBlendState::Count; 
	//	eSamplerState samplerState = eSamplerState::Count;

		uint32_t stencilRef = 0;
		float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		uint32_t sampleMask = 0xFFFFFFFF;
	};
}