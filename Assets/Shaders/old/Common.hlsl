#include "CommonBuffers.hlsl"
#include "InputLayout.hlsl"
#include "ConstantBuffers.hlsl"
#include "Samplers.hlsl"

float3 PhongDiffuse(float3 normal, float3 color)
{
	float NDotL = dot(-cbLightDirPS, normal);
	return color * saturate(NDotL);
}

float3 BlinnSpecular(float3 viewDir, float3 normal, float3 color, float specPow, float specInt)
{
	float3 halfWay = normalize(viewDir + -cbLightDirPS);
	float NDotH = saturate(dot(halfWay, normal));

	return color * pow(NDotH, specPow) * specInt;
}


matrix GetSkinMatrix(uint4 blendIndices, float4 blendWeights)
{
	return cbSkinMatrix[blendIndices.x] * blendWeights.x
		+ cbSkinMatrix[blendIndices.y] * blendWeights.y
		+ cbSkinMatrix[blendIndices.z] * blendWeights.z
		+ cbSkinMatrix[blendIndices.w] * blendWeights.w;
}