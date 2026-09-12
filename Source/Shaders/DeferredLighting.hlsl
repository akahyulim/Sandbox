#include "Constants.hlsli"
#include "Resources.hlsli"

static const float2 arrBasePos[4] =
{
    float2(-1.0, 1.0),
	float2(1.0, 1.0),
	float2(-1.0, -1.0),
	float2(1.0, -1.0),
};

struct VSToPS
{
    float4 position : SV_POSITION;
    float2 cpPos : TEXCOORD0;
};

VSToPS MainVS(uint VertexID : SV_VERTEXID)
{
    VSToPS output;

    output.position = float4(arrBasePos[VertexID].xy, 0.0, 1.0);
    output.cpPos = output.position.xy;

    return output;
}

float4 MainPS(VSToPS input) : SV_Target
{
    int3 location3 = int3(input.position.xy, 0);
    float3 albedo = GBuffer_AlbedoRoughness.Load(location3).xyz;
    uint objectID = GBuffer_ObjectID.Load(location3);

    // 계층구조일 땐 방법을 달리해야한다.
    if (objectID != 0 && objectID == selectedObject.id)
    {
        albedo = lerp(albedo, float3(1.0f, 0.8f, 0.2f), 0.3f);
    }
    return float4(albedo, 1.0f);
}