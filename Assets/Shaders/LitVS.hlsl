#include "VSInput.hlsli"
#include "Constants.hlsli"

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
    float3 WorldPos : TEXCOORD1;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 BiNormal : BINORMAL;
};

VSOutput MainVS(VSInput_Lit input)
{
    VSOutput output;

    float4 position = input.Position;
    position.w = 1.0f;
    
    output.Position = mul(position, cbObjectVS.worldMatrix);
    output.WorldPos = output.Position.xyz;
    output.Position = mul(output.Position, cbCamera.viewProjMatrix);

    output.UV = input.UV;

    output.Normal = mul(input.Normal, (float3x3)cbObjectVS.worldMatrix);
    output.Normal = normalize(output.Normal);
    output.Tangent = mul(input.Tangent, (float3x3)cbObjectVS.worldMatrix);
    output.Tangent = normalize(output.Tangent);
    output.BiNormal = mul(input.BiNormal, (float3x3)cbObjectVS.worldMatrix);
    output.BiNormal = normalize(output.BiNormal);
    
    return output;
}