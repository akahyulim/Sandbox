#include "Constants.hlsli"

struct VSToPS
{
    float4 Pos : SV_POSITION;
    float3 RayDir : TEXCOORD0;
};

float4 MainPS(VSToPS input) : SV_Target
{
    float3 dir = normalize(input.RayDir);
    float factor = saturate(dir.y);  
    float3 finalColor = lerp(weatherData.horizonColor.rgb, weatherData.apexColor.rgb, factor);

    return float4(finalColor, 1.0f);
}