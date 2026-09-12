#include "Constants.hlsli"

struct VSToPS
{
    float4 Pos : SV_POSITION;
    float3 PosL : POSITION;
};

float4 MainPS(VSToPS input) : SV_Target
{
    return float4(weatherData.skyColor.xyz, 1.0f);
}