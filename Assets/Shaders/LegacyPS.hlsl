#include "Constants.hlsli"
#include "Resources.hlsli"

float3 CalcuDirLight(float3 worldPos, float3 normal, LightData light)
{
    float3 finalColor = {0.0, 0.0, 0.0};
    float3 lightColor = light.color;
    
    // Phong diffuse
    float NDotL = saturate(dot(-light.direction, normal));
    if (NDotL > 0.0f)
    {
        finalColor += lightColor * NDotL;
    }

	// Blinn specular
    float3 toEye = gCamera.position.xyz - worldPos;
    toEye = normalize(toEye);
    float3 halfWay = normalize(toEye + -light.direction);
    float NDotH = saturate(dot(halfWay, normal));
    
    finalColor += lightColor * pow(NDotH, 250.0f) * 0.25f;
    
    return finalColor;
}

float3 CalcuPointLight(float3 worldPos, float3 normal, LightData light)
{
    float3 lightColor = light.color;
    float3 toLight = light.position - worldPos;
    float3 toEye = gCamera.position.xyz - worldPos;
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

float3 CalcuSpotLight(float3 worldPos, float3 normal, LightData light)
{
    float3 lightColor = light.color; // 🌟 C++에서 Intensity가 이미 곱해진 순수 광원 색상
    
    // 1. 거리 및 기본 방향 벡터 계산
    float3 toLight = light.position - worldPos;
    float distance = length(toLight);
    float3 lightDir = normalize(toLight);

    // 🌟 [최적화 1] 거리 기반 감쇠 및 조기 차단 (Early Out)
    // 빛의 유효 범위를 벗어났다면 뒤의 무거운 스폿 각도/스펙큘러 연산을 할 필요가 없습니다.
    float distNorm = 1.0f - saturate(distance * light.rangeRcp);
    float attn = distNorm * distNorm;
    if (attn <= 0.0f) 
        return float3(0.0f, 0.0f, 0.0f);

    // 2. 스폿 감쇠 인자(Spot Factor) 계산
    // 표면에서 광원을 바라보는 벡터(lightDir)와 빛이 쏘아지는 방향(-light.direction) 내적
    float spotFactor = dot(lightDir, -light.direction);

    // 🌟 [최적화 2] C++에서 이미 계산되어 넘어온 코사인 값을 그대로 사용합니다!
    // ❌ float innerCutoff = cos(light.innerAngle); -> 제거 (삼각함수 오버헤드 삭제)
    float innerCutoff = light.cosInnerAngle;
    float outerCutoff = light.cosOuterAngle;

    // 3. 스폿 범위에 따른 페이드아웃(Falloff) 연산
    float spotAttn = saturate((spotFactor - outerCutoff) / (innerCutoff - outerCutoff));
    spotAttn *= spotAttn; // 경계면을 부드럽게 스무딩

    // 🌟 [최적화 3] 원추(Cone) 범위를 완전히 벗어난 픽셀이면 즉시 리턴
    if (spotAttn <= 0.0f)
        return float3(0.0f, 0.0f, 0.0f);

    // 4. Phong Diffuse 연산
    float NDotL = saturate(dot(lightDir, normal));
    float3 finalColor = lightColor * NDotL;

    // 5. Blinn-Phong Specular 연산
    float3 toEye = normalize(gCamera.position.xyz - worldPos);
    float3 halfWay = normalize(toEye + lightDir);
    float NDotH = saturate(dot(halfWay, normal));
    finalColor += lightColor * pow(NDotH, 250.0f) * 0.25f;

    // 6. 최종 감쇄 적용 (거리 감쇄 * 스폿 송곳 모양 감쇄)
    return finalColor * (attn * spotAttn);
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
        DiffuseMap.Sample(WrapLinearSampler, input.UV) * gMaterial.diffuseColor : gMaterial.diffuseColor;

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
/*
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
*/
    float3 lightColor = float3(0.1, 0.1, 0.1);
    lightColor += CalcuDirLight(input.WorldPos, normal, gLight);

    return float4(mtrlColor.rgb * lightColor.rgb, mtrlColor.a);
}
