#include "Resources.hlsli"

struct PSInput
{
    float4 position : SV_POSITION;
    float2 cpPos : TEXCOORD0;
};

float4 MainPS(PSInput input) : SV_Target
{
    float2 uv = input.cpPos * 0.5 + 0.5;
    uv.y = 1.0 - uv.y; // Y축 반전
    
    return SceneColor.Sample(ClampLinearSampler, uv);
}