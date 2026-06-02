#include "Common.hlsl"

struct VS_INPUT
{
    float4 position : POSITION0;
    float2 tex : TEXCOORD0;
    float3 normalTex : NORMAL0;
    float3 tangent : TANGENT0;
    float3 bitangent : BINORMAL0;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 normalTex : NORMAL0;
    float3 tangent : TANGENT0;
    float3 bitangent : BINORMAL0;
};

VS_OUTPUT MainVS(VS_INPUT input)
{
    VS_OUTPUT output;
    
    input.position.w = 1.0f;
    output.position = mul(input.position, ObjectWorld); //cbModelVertex.world);
    output.worldPos = output.position.xyz;
    output.position = mul(output.position, mul(CameraView, CameraProj)); //cbCameraVertex.view, cbCameraVertex.projection));
    output.tex = input.tex;
    output.normalTex = mul(input.normalTex, (float3x3) ObjectWorld); //cbModelVertex.world);
    output.normalTex = normalize(output.normalTex);
    output.tangent = mul(input.tangent, (float3x3) ObjectWorld); //cbModelVertex.world);
    output.tangent = normalize(output.tangent);
    output.bitangent = mul(input.bitangent, (float3x3) ObjectWorld); // cbModelVertex.world);
    output.bitangent = normalize(output.bitangent);
    
    return output;
}

struct PS_GBUFFER
{
	float4 diffuseTex : SV_TARGET0;
	float4 normalTex : SV_TARGET1;
	float4 specularTex : SV_TARGET2;
};

PS_GBUFFER MainPS(VS_OUTPUT input)
{
	PS_GBUFFER output;

    // diffuse
    float4 diff;
    if (!HasDiffuseTexture())
        diff = MaterialDiff; //cbModelPixel.color;
    else
        diff = DiffuseMap.Sample(LinearSampler, input.tex);
    diff *= diff; // linear space
	output.diffuseTex.xyz = diff.xyz;

    // normal
    float3 normalTex = input.normalTex;
    if (HasNormalTexture())
    {
        float4 bumpMap = NormalMap.Sample(LinearSampler, input.tex);
        bumpMap = (bumpMap * 2.0f) - 1.0f;

        normalTex = normalize((bumpMap.x * input.tangent) + (bumpMap.y * input.bitangent) + (bumpMap.z * input.normalTex));
    }
	output.normalTex = float4(normalTex * 0.5f + 0.5f, 0.0);

    return output;
}