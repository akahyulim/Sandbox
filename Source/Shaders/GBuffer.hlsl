#include "Constants.hlsli"
#include "Resources.hlsli"

struct VSInput
{
    float4 Position : POSITION;
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
    
    float4 position = input.Position;
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

struct PSOutput
{
    float4 AlbedoRoughness : SV_TARGET0;
    float4 NormalMetallic : SV_TARGET1;
    float4 Emissive : SV_TARGET2;
    uint ObjectID : SV_TARGET3;
};

PSOutput MainPS(VSToPS input)
{
    PSOutput output;
    
    float2 uv = (input.UV * materialData.tiling) + materialData.offset;
    
    float3 albedo = materialData.baseColor;
    if (HasAlbedoMap())
    {
        albedo = AlbedoMap.Sample(WrapLinearSampler, uv).xyz;
        //albedo *= albedo;
    }
    output.AlbedoRoughness = float4(albedo, 0.0f);
    
    float3 normal = input.Normal;
    if (HasNormalMap())
    {
        float4 bumpMap = NormalMap.Sample(WrapLinearSampler, uv);
        bumpMap = (bumpMap * 2.0f) - 1.0f;

        normal = normalize((bumpMap.x * input.Tangent) + (bumpMap.y * input.BiNormal) + (bumpMap.z * input.Normal));
    }
    output.NormalMetallic = float4(normal * 0.5f + 0.5f, 0.0f);
    
    output.Emissive = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    output.ObjectID = objectData.id;
    
    return output;
}