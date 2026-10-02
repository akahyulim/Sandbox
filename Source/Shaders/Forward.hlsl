#include "Constants.hlsli"
#include "Resources.hlsli"
#include "Lighting.hlsli"

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
    float4 ldrRtv : SV_TARGET0;
    uint ObjectID : SV_TARGET1;
};

PSOutput MainPS(VSToPS input)
{   
    float2 uv = (input.UV * materialData.tiling) + materialData.offset;
    
    float4 baseColor = materialData.baseColor;
    float3 albedoColor = baseColor.xyz;
    float finalAlpha = baseColor.w;

    if (HasAlbedoMap())
    {
        float4 texColor = AlbedoMap.Sample(WrapLinearSampler, uv);
    
        // 1. 색상에 텍스처 컬러 곱하고 감마 보정(제곱) 적용
        albedoColor *= texColor.xyz;
        albedoColor *= albedoColor;
    
        // 2. 알파는 제곱하지 않고 원본 비율 그대로 곱하기
        finalAlpha *= texColor.w;
    }
    
    float roughness = materialData.roughnessFactor;
    float metallic = materialData.metallicFactor;

    if (HasORMMap())
    {
        float3 orm = ORMMap.Sample(WrapLinearSampler, uv).xyz;
        //orm.r (AO는 필요에 따라 Ambient Occlusion 렌더 타겟에 쓸 수 있음)
        roughness *= orm.g;
        metallic *= orm.b;
    }
    
    float3 normal = input.Normal;
    if (HasNormalMap())
    {
        float4 bumpMap = NormalMap.Sample(WrapLinearSampler, uv);
        bumpMap = (bumpMap * 2.0f) - 1.0f;

        normal = normalize((bumpMap.x * input.Tangent) + (bumpMap.y * input.BiNormal) + (bumpMap.z * input.Normal));
    }
    
    float4 emissive = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    float3 V = normalize(frameData.cameraPosition.xyz - input.WorldPos);
    
    // 1. 기본 앰비언트(환경광) 계산
    float3 ambientColor = weatherData.ambientColor.xyz;
    float3 ambient = albedoColor * ambientColor;

    // 2. 다중 라이트(디렉셔널, 포인트, 스팟) 누적 연산
    float3 totalPbrLighting = 0.0f;

    for (uint i = 0; i < lights.lightCount; ++i)
    {
        LightData light = lights.lights[i];
        
        float3 lightDir = 0.0f;
        float attenuation = 1.0f;
        
        if (light.type == 0) // Directional Light
        {
            lightDir = normalize(-light.direction);
            attenuation = 1.0f;
        }
        else if (light.type == 1) // Point Light
        {
            float3 lightToPixel = light.position - input.WorldPos;
            float distance = length(lightToPixel);
            
            // 범위를 벗어나면 연산 생략 (최적화)
            float range = 1.0f / light.rangeRcp;
            if (distance > range) 
                continue;
                
            lightDir = lightToPixel / distance;
            
            // 거리 감쇠 계산 (Standard Clamped Distance Attenuation)
            float distanceNorm = distance * light.rangeRcp;
            float atten = saturate(1.0f - (distanceNorm * distanceNorm));
            attenuation = atten * atten;
        }
        else if (light.type == 2) // Spot Light
        {
            float3 lightToPixel = light.position - input.WorldPos;
            float distance = length(lightToPixel);
            
            float range = 1.0f / light.rangeRcp;
            if (distance > range) 
                continue;
                
            lightDir = lightToPixel / distance;
            
            // 포인트 거리 감쇠
            float distanceNorm = distance * light.rangeRcp;
            float pointAtten = saturate(1.0f - (distanceNorm * distanceNorm));
            pointAtten = pointAtten * pointAtten;
            
            // 스팟 콘(원뿔) 각도 감쇠
            float cosTheta = dot(-lightDir, light.direction);
            float spotAtten = saturate((cosTheta - light.cosOuterAngle) / (light.cosInnerAngle - light.cosOuterAngle));
            
            attenuation = pointAtten * spotAtten;
        }
        
        // PBR BRDF 함수 적용 후 빛의 색상 및 감쇠 반영
        float3 radiance = BRDF_PBR(normal, V, lightDir, albedoColor, metallic, roughness) * light.color * light.intensity * attenuation;
        totalPbrLighting += radiance;
    }

    // 3. 최종 색상 조합 (앰비언트 + 모든 라이트의 합)
    float3 finalColor = ambient + totalPbrLighting;
   
    // 계층구조일 땐 방법을 달리해야한다.
    if (objectData.id != 0 && objectData.id == selectedObject.id)
    {
        finalColor = lerp(finalColor, float3(1.0f, 0.8f, 0.2f), 0.3f);
    }
    
    // 감마 보정
    finalColor = pow(finalColor, 1.0 / 2.2);
    
    PSOutput output;
    output.ObjectID = objectData.id;
    output.ldrRtv = float4(finalColor, finalAlpha);
    
    return output;
}