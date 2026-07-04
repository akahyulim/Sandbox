#pragma once
#include "Resource.h"
#include "ShaderProgram.h"
#include "Core/Types.h"

namespace Dive
{
	struct DvPipelineState : public Resource
	{
		ePrimitiveTopology topology = ePrimitiveTopology::None;

		std::shared_ptr<ShaderProgram> shaderProgram = nullptr;

		eDepthStencilState depthStencilState = eDepthStencilState::Count;
		eRasterizerState rasterizerState = eRasterizerState::Count;
		eBlendState blendState = eBlendState::Count;

		bool operator==(const DvPipelineState& other) const
		{
			return topology == other.topology &&
				shaderProgram == other.shaderProgram &&
				depthStencilState == other.depthStencilState &&
				rasterizerState == other.rasterizerState &&
				blendState == other.blendState;
		}
	};
}
