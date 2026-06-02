#include "Common.hlsl"

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float4 domePosition : TEXCOORD1;
};

PixelInput MainVS(Vertex_PosTexNorTan input)
{
    PixelInput output;

    input.position.w = 1.0f;
    output.position = mul(input.position, cbCameraMatrixVS);
    output.position = mul(output.position, cbViewMatrixVS);
    output.position = mul(output.position, cbProjMatrixVS);

    output.tex = input.tex;
    
    output.domePosition = input.position;
    
    return output;
}

struct PS_GBUFFER
{
    float4 colorSpecIntensity : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 specPower : SV_TARGET2;
};

cbuffer SkydomePS : register(b0)
{
    float4 apexColor;
    float4 centerColor;
}

PS_GBUFFER MainPS(PixelInput input)
{
    PS_GBUFFER output;
    
    float height = input.domePosition.y;
    if(height  < 0.0f)
        height = 0.0f;
 
    output.colorSpecIntensity = lerp(centerColor, apexColor, height);
    
    return output;
}