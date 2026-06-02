#include "Constants.hlsli"

float3 GetSkyboxVertexDirection(uint vertexID)
{
    static const float3 directions[36] = {
        // +X (Right)
        float3(1, -1, 1), float3(1, 1, 1), float3(1, 1, -1),
        float3(1, -1, 1), float3(1, 1, -1), float3(1, -1, -1),

        // -X (Left)
        float3(-1, -1, -1), float3(-1, 1, -1), float3(-1, 1, 1),
        float3(-1, -1, -1), float3(-1, 1, 1), float3(-1, -1, 1),

        // +Y (Top)
        float3(-1, 1, 1), float3(-1, 1, -1), float3(1, 1, -1),
        float3(-1, 1, 1), float3(1, 1, -1), float3(1, 1, 1),

        // -Y (Bottom)
        float3(-1, -1, -1), float3(-1, -1, 1), float3(1, -1, 1),
        float3(-1, -1, -1), float3(1, -1, 1), float3(1, -1, -1),

        // +Z (Front)
        float3(-1, -1, 1), float3(-1, 1, 1), float3(1, 1, 1),
        float3(-1, -1, 1), float3(1, 1, 1), float3(1, -1, 1),

        // -Z (Back)
        float3(1, -1, -1), float3(1, 1, -1), float3(-1, 1, -1),
        float3(1, -1, -1), float3(-1, 1, -1), float3(-1, -1, -1),
    };
    return directions[vertexID];
}

struct VSOut
{
    float4 position : SV_Position;
    float3 direction : TEXCOORD;
};

VSOut MainVS(uint vertexID : SV_VertexID)
{
    VSOut output;

    float3 dir = GetSkyboxVertexDirection(vertexID);

    matrix viewNoTranslation = cbCamera.viewMatrix;
    viewNoTranslation._41 = 0;
    viewNoTranslation._42 = 0;
    viewNoTranslation._43 = 0;

    float4 worldPos = mul(float4(dir, 1.0f), viewNoTranslation);
    output.position = mul(worldPos, cbCamera.projMatrix);
    output.direction = normalize(dir);

    return output;
}
