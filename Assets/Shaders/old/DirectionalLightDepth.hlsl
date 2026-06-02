#include "Common.hlsl"

// 여기에서 월드변환을 하고
// gs에서 각각의 viewProj변환을 하는 게 맞는 것 같다.
float4 MainVS(float4 pos : POSITION0) : SV_Position
{
    pos.w = 1.0;
    return mul(pos, ObjectWorld);
}

cbuffer cbGeometry : register(b2)
{
    float4x4 CascadeViewProj[3] : packoffset(c0);
}

struct GS_OUTPUT
{
    float4 pos : SV_Position;
    uint rtIndex : SV_RenderTargetArrayIndex;
};

[maxvertexcount(9)]
void MainGS(triangle float4 inPos[3] : SV_Position, inout TriangleStream<GS_OUTPUT> outStream)
{
    for (int face = 0; face < 3; face++)
    {
        GS_OUTPUT output;
        output.rtIndex = face;
        
        for (int v = 0; v < 3; v++)
        {
            output.pos = mul(inPos[v], CascadeViewProj[face]);
            outStream.Append(output);
        }
        outStream.RestartStrip();
    }
}