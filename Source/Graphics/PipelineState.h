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
		
		std::shared_ptr<ShaderProgram> shaderProgram = nullptr;
		
		eDepthStencilState depthStencilState = eDepthStencilState::Count;
		eRasterizerState rasterizerState = eRasterizerState::Count;
		eBlendState blendState = eBlendState::Count; 

		bool operator==(const PipelineState& other) const
		{
			return topology == other.topology &&
				shaderProgram == other.shaderProgram &&
				depthStencilState == other.depthStencilState &&
				rasterizerState == other.rasterizerState &&
				blendState == other.blendState;
		}

		// 아래의 값들은 변경이 필요할 수 있다.
		// 분리하는 편이 낫다.
		uint32_t stencilRef = 0;
		float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		uint32_t sampleMask = 0xFFFFFFFF;
	};
}