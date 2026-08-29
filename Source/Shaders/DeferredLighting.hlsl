#include "Constants.hlsli"
#include "Resources.hlsli"

static const float2 arrBasePos[4] =
{
    float2(-1.0, 1.0),
	float2(1.0, 1.0),
	float2(-1.0, -1.0),
	float2(1.0, -1.0),
};

struct VSToPS
{
    float4 position : SV_POSITION;
    float2 cpPos : TEXCOORD0;
};

VSToPS MainVS(uint VertexID : SV_VERTEXID)
{
    VSToPS output;

    output.position = float4(arrBasePos[VertexID].xy, 0.0, 1.0);
    output.cpPos = output.position.xy;

    return output;
}

float4 MainPS(VSToPS input) : SV_Target
{
    int3 location3 = int3(input.position.xy, 0);
    return float4(GBuffer_AlbedoRoughness.Load(location3).xyz, 1.0);
    
    //float2 uv = input.cpPos * 0.5 + 0.5;
    //uv.y = 1.0 - uv.y; // Y축 반전
    
    //return GBuffer_AlbedoRoughness.Sample(ClampLinearSampler, uv);
}