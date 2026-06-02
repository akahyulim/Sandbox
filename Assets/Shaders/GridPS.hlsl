#include "Constants.hlsli"

struct PSInput
{
    float4 position : SV_Position;
    float3 worldPos : TEXCOORD0;
};

float4 MainPS(PSInput input) : SV_Target
{
    float gridScale = 1.0;
    float lineThickness = 0.02;
    float axisThickness = 0.05;

    float4 xColor = float4(1.0, 0.0, 0.0, 1.0); // X축 강조색
    float4 zColor = float4(0.0, 0.0, 1.0, 1.0); // Z축 강조색
    float4 lineColor = float4(1.0, 1.0, 1.0, 1.0); // 일반 격자선

    float2 gridUV = input.worldPos.xz * gridScale;
    float2 grid = abs(frac(gridUV) - 0.5);
    float lines = step(lineThickness, min(grid.x, grid.y));

    // 중심축 강조 조건 수정
    float xAxis = step(abs(input.worldPos.x), axisThickness);
    float zAxis = step(abs(input.worldPos.z), axisThickness);

    // 색상 우선순위 처리
    float4 color = cbCamera.backgroundColor;
    if (lines < 0.5)
        color = lineColor;
    if (zAxis > 0.5)
        color = zColor;
    if (xAxis > 0.5)
        color = xColor;

    return color;
}