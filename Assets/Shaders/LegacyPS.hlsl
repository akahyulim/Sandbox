#include "Constants.hlsli"
#include "Resources.hlsli"

float3 CalcuDirLight(float3 worldPos, float3 normal, Light light)
{
    float3 finalColor = {0.0, 0.0, 0.0};
    float3 lightColor = light.color * light.intensity;
    
    // Phong diffuse
    float NDotL = saturate(dot(-light.direction, normal));
    if (NDotL > 0.0f)
    {
        finalColor += lightColor * NDotL;
    }

	// Blinn specular
    float3 toEye = cbCamera.position.xyz - worldPos;
    toEye = normalize(toEye);
    float3 halfWay = normalize(toEye + -light.direction);
    float NDotH = saturate(dot(halfWay, normal));
    
    finalColor += lightColor * pow(NDotH, 250.0f) * 0.25f;
    
    return finalColor;
}

float3 CalcuPointLight(float3 worldPos, float3 normal, Light light)
{
    float3 lightColor = light.color * light.intensity;
    float3 toLight = light.position - worldPos;
    float3 toEye = cbCamera.position.xyz - worldPos;
    float distance = length(toLight);
    
    // phong diffuse
    toLight /= distance;
    float NDotL = saturate(dot(toLight, normal));
    float3 finalColor = lightColor * NDotL;
    
    // blinn specular
    toEye = normalize(toEye);
    float3 halfWay = normalize(toEye + toLight);
    float NDotH = saturate(dot(halfWay, normal));
    finalColor += lightColor * pow(NDotH, 250.0f) * 0.25f;
    
    // attenuation
    float distToLightNorm = 1.0f - saturate(distance * light.rangeRcp);
    float attn = distToLightNorm * distToLightNorm;
    
    finalColor *= attn;
    return finalColor;
}

float3 CalcuSpotLight(float3 worldPos, float3 normal, Light light)
{
    float3 lightColor = light.color * light.intensity;
    float3 toLight = light.position - worldPos;
    float distance = length(toLight);
    float3 lightDir = normalize(toLight);

    // Spot factor: 픽셀 방향과 빛 방향 사이의 각도
    float spotFactor = dot(lightDir, -light.direction);

    // Inner/Outer cutoff
    float innerCutoff = cos(light.innerAngle);
    float outerCutoff = cos(light.outerAngle);

    // Spot attenuation 계산
    float spotAttn = saturate((spotFactor - outerCutoff) / (innerCutoff - outerCutoff));
    spotAttn *= spotAttn; // falloff 부드럽게

    // 조명 영향 없음
    if (spotAttn <= 0.0f)
        return float3(0.0, 0.0, 0.0);

    // Phong diffuse
    float NDotL = saturate(dot(lightDir, normal));
    float3 finalColor = lightColor * NDotL;

    // Blinn specular
    float3 toEye = normalize(cbCamera.position.xyz - worldPos);
    float3 halfWay = normalize(toEye + lightDir);
    float NDotH = saturate(dot(halfWay, normal));
    finalColor += lightColor * pow(NDotH, 250.0f) * 0.25f;

    // 거리 기반 감쇠
    float distNorm = 1.0f - saturate(distance * light.rangeRcp);
    float attn = distNorm * distNorm;

    // 최종 색상
    finalColor *= attn * spotAttn;
    return finalColor;
}

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
    float3 WorldPos : TEXCOORD1;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 BiNormal : BINORMAL;
};

float4 MainPS(PSInput input) : SV_TARGET
{
    float4 mtrlColor = HasDiffuseMap() ?
        DiffuseMap.Sample(WrapLinearSampler, input.UV) * cbMaterialPS.diffuseColor : cbMaterialPS.diffuseColor;

    float3 normal = input.Normal;
    if (HasNormalMap())
    {
       // 노멀 맵 샘플링
        float3 bumpMap = NormalMap.Sample(WrapLinearSampler, input.UV).xyz;
        bumpMap = bumpMap * 2.0f - 1.0f; // [-1, 1] 범위로 변환

        // TBN 행렬 구성 (이미 월드 공간 기준)
        float3x3 TBN = float3x3(
            input.Tangent,
            input.BiNormal,
            input.Normal
        );

        // Tangent Space → World Space 변환
        normal = normalize(mul(bumpMap, TBN));
    }

    float3 lightColor = cbForwardLightPS.ambientColor;
    for (int i = 0; i < cbForwardLightPS.lightCount; ++i)
    {
        switch (cbForwardLightPS.lights[i].type)
        {
            case 0: lightColor += CalcuDirLight(input.WorldPos, normal, cbForwardLightPS.lights[i]); break;
            case 1: lightColor += CalcuPointLight(input.WorldPos, normal, cbForwardLightPS.lights[i]); break;
            case 2: lightColor += CalcuSpotLight(input.WorldPos, normal, cbForwardLightPS.lights[i]); break;
        }
    }

    return float4(mtrlColor.rgb * lightColor.rgb, mtrlColor.a);
}
