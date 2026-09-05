#include "Constants.hlsli"
#include "Resources.hlsli"

struct VSInput
{
    float4 PosL : POSITION;
    float2 UV : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 BiNormal : BINORMAL;
};

struct VSToPS
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
    float3 WorldPos : TEXCOORD1;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 BiNormal : BINORMAL;
};

VSToPS MainVS(VSInput input)
{
    VSToPS output;
    
    float4 position = input.PosL;
    position.w = 1.0f;
    
    output.Position = mul(position, objectData.model);
    output.WorldPos = output.Position.xyz;
    output.Position = mul(output.Position, frameData.viewProjection);

    output.UV = input.UV;
    
    output.Normal = mul(input.Normal, (float3x3) objectData.model);
    output.Normal = normalize(output.Normal);
    output.Tangent = mul(input.Tangent, (float3x3) objectData.model);
    output.Tangent = normalize(output.Tangent);
    output.BiNormal = mul(input.BiNormal, (float3x3) objectData.model);
    output.BiNormal = normalize(output.BiNormal);
    return output;
}

float4 MainPS(VSToPS input) : SV_Target
{
    float2 uv = (input.UV * materialData.tiling) + materialData.offset;

    return HasAlbedoMap() ?
       AlbedoMap.Sample(WrapLinearSampler, uv) : materialData.baseColor;
}
/*
struct VSToPS
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
};

VSToPS MainVS(VSInput input)
{
    VSToPS output = (VSToPS) 0;
    output.Pos = mul(mul(input.PosL, objectData.model), frameData.viewProjection);
    output.UV = input.UV;
    return output;
}

float4 MainPS(VSToPS input) : SV_Target
{
    return HasAlbedoMap() ?
       AlbedoMap.Sample(WrapLinearSampler, input.UV) * materialData.baseColor : materialData.baseColor;
}
*/