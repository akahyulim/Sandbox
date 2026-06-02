#include "Common.hlsl"

struct PixelInput
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 worldPos : TEXCOORD1;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
};

PixelInput MainVS(Vertex_Skinned input)
{
	PixelInput output;
	float4 animatedPos;
	input.position.w = 1.0f;
	
	for (int i = 0; i < 4; ++i)
	{
		if (input.boneIDs[i] >= 100)
			continue;

		float4 localPosition = mul(input.position, cbSkinMatrix[input.boneIDs[i]]);
		animatedPos += localPosition * input.boneWeights[i];
	}
	
	output.position = mul(animatedPos, cbWorldMatrixVS);
	output.position = mul(output.position, cbViewMatrixVS);
	output.position = mul(output.position, cbProjMatrixVS);
	
	output.worldPos = mul(animatedPos, cbWorldMatrixVS).xyz;

	output.tex = input.tex;
	
	output.normal = mul(input.normal, (float3x3)cbWorldMatrixVS);
	output.normal = normalize(output.normal);
	
	output.tangent = mul(input.tangent, (float3x3)cbWorldMatrixVS);
	output.tangent = normalize(output.tangent);

	return output;
}

struct PS_GBUFFER
{
	float4 colorSpecIntensity : SV_TARGET0;
	float4 normal : SV_TARGET1;
	float4 specPower : SV_TARGET2;
};

PS_GBUFFER MainPS(PixelInput input)
{
	PS_GBUFFER output;
	
    float4 diffuse = has_texture_diffse() ? DiffuseMap.Sample(BaseSampler, input.tex) : cbMaterialAlbedoColorPS;
	diffuse *= diffuse;	// ¿Ö Á¦°ö?
	output.colorSpecIntensity = diffuse;

	output.normal = float4(input.normal * 0.5 + 0.5, 0.0);

	return output;
}