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
    //float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    //color.xy = frameData.mousePosition / frameData.screenResolution;
    //color.z = 1.0f;  
    //return color;
    
    int3 location3 = int3(input.position.xy, 0);
    return float4(GBuffer_AlbedoRoughness.Load(location3).xyz, 1.0);
}