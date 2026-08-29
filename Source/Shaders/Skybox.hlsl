#include "Constants.hlsli"
#include "Resources.hlsli"

struct VSInput
{
    float3 PosL : POSITION;
};

struct VSToPS
{
    float4 Pos : SV_POSITION;
    float3 PosL : POSITION;
};

VSToPS MainVS(VSInput input)
{
    VSToPS output = (VSToPS) 0;
    output.Pos = mul(mul(float4(input.PosL, 1.0f), objectData.model), frameData.viewProjMatrix).xyww;
    output.PosL = input.PosL;
    return output;
}

float4 MainPS(VSToPS input) : SV_Target
{
    return Skymap.Sample(SkyboxSampler, input.PosL);
}