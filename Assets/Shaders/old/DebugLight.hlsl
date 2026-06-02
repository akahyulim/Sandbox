#include "Common.hlsl"

struct DS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 cpPos : TEXCOORD0;
};

cbuffer cbPixel : register(b2)
{
    float3 LightColor               : packoffset(c0);
    float LightRangeRcp             : packoffset(c0.w);
    float3 LightPos                 : packoffset(c1);
    uint ShadowEnabled              : packoffset(c1.w);
    float2 LightPerspectiveValues   : packoffset(c2);
};

float4 MainPS(DS_OUTPUT input) : SV_TARGET0
{
    return float4(LightColor, 1.0);
}