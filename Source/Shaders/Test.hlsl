#include "Constants.hlsli"
#include "Resources.hlsli"

static const float3 PosL[3] =
{
    float3(-1, -1, 0),
    float3(0, 1, 0),
    float3(1, -1, 0)
};

struct VSToPS
{
    float4 Pos : SV_POSITION;
};

VSToPS MainVS(uint vertexID : SV_VertexID)
{
    VSToPS output = (VSToPS) 0;
    output.Pos = mul(mul(float4(PosL[vertexID], 1.0f), objectData.model), frameData.viewProjMatrix);
    return output;
}

float4 MainPS(VSToPS input) : SV_Target
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}