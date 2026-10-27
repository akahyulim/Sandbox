#include "Constants.hlsli"

struct VSToPS
{
    float4 Pos : SV_POSITION;
    float3 RayDir : TEXCOORD0;
};

float4 MainPS(VSToPS input) : SV_Target
{
    float3 dir = normalize(input.RayDir);
    
    float3 wallColor = environmentData.apexColor.rgb;
    float3 floorColor = environmentData.horizonColor.rgb;

    // dir.y는 -1(바닥 밑)부터 +1(천장 위)까지의 값
    float y = dir.y;

    // [핵심] if-else 분기를 없애고, 부호(sign)를 유지한 채 지수 함수 곡선을 적용합니다.
    // 지수(Exponent)를 조절하여 수평선 코너 부근의 곡면 부드러움(안티/스무딩)을 제어할 수 있습니다.
    float curve = sign(y) * pow(abs(y), 0.6f);

    // -1 ~ 1 범위를 0 ~ 1로 정규화
    float t = curve * 0.5f + 0.5f;

    // 바닥과 벽 색상을 뚝 끊김 없이 하나의 부드러운 곡선(Cyclorama)으로 보간합니다.
    float3 finalColor = lerp(floorColor, wallColor, t);

    return float4(finalColor, 1.0f);
}
/*
float4 MainPS(VSToPS input) : SV_Target
{
    float3 dir = normalize(input.RayDir);
    float factor = saturate(dir.y);  
    float3 finalColor = lerp(environmentData.horizonColor.rgb, environmentData.apexColor.rgb, factor);

    return float4(finalColor, 1.0f);
}
*/