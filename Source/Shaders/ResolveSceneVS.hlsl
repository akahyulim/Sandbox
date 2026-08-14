static const float2 arrBasePos[4] =
{
    float2(-1.0, 1.0),
	float2(1.0, 1.0),
    float2(-1.0, -1.0),
	float2(1.0, -1.0),
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 cpPos : TEXCOORD0;
};

VSOutput MainVS(uint VertexID : SV_VERTEXID)
{
    VSOutput output;

    output.position = float4(arrBasePos[VertexID].xy, 0.0, 1.0);
    output.cpPos = output.position.xy;

    return output;
}