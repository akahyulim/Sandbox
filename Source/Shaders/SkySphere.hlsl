#include "Constants.hlsli"
#include "Resources.hlsli"

struct VSToPS
{
    float4 Pos : SV_POSITION;
    float3 RayDir : TEXCOORD0;
};

VSToPS MainVS(uint vertexID : SV_VertexID)
{
    VSToPS output = (VSToPS) 0;

    float2 grid = float2((vertexID == 2) ? 3.0f : -1.0f, (vertexID == 1) ? 3.0f : -1.0f);
    output.Pos = float4(grid, 1.0f, 1.0f);

    float4 unprojected = mul(float4(grid, 1.0f, 1.0f), frameData.inverseViewProjection);
    float3 worldPos = unprojected.xyz / unprojected.w;
    output.RayDir = worldPos - frameData.cameraPosition.xyz;

    return output;
}

float2 DirectionToSphericalUV(float3 dir)
{
    float3 normDir = normalize(dir);
    float u = atan2(normDir.z, normDir.x) * (1.0f / (2.0f * 3.14159265f)) + 0.5f;
    float v = asin(-normDir.y) * (1.0f / 3.14159265f) + 0.5f;
    return float2(u, v);
}

float4 MainPS(VSToPS input) : SV_Target
{
    float2 uv = DirectionToSphericalUV(input.RayDir);
    float3 color = SkySphere.Sample(SkySphereSampler, uv).rgb;

    return float4(color, 1.0f);
}