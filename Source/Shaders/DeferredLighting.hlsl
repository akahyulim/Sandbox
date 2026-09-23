#include "Constants.hlsli"
#include "Resources.hlsli"

static const float2 arrBasePos[4] =
{
    float2(-1.0, 1.0),
	float2(1.0, 1.0),
	float2(-1.0, -1.0),
	float2(1.0, -1.0),
};

struct VSToPS
{
    float4 position : SV_POSITION;
    float2 cpPos : TEXCOORD0;
};

VSToPS MainVS(uint VertexID : SV_VERTEXID)
{
    VSToPS output;

    output.position = float4(arrBasePos[VertexID].xy, 0.0, 1.0);
    output.cpPos = output.position.xy;

    return output;
}

static const float PI = 3.14159265359f;

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = saturate(dot(N, H));
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float NdotV = saturate(dot(N, V));
    float NdotL = saturate(dot(N, L));

    float gv = NdotV / (NdotV * (1.0 - k) + k);
    float gl = NdotL / (NdotL * (1.0 - k) + k);

    return gv * gl;
}

float3 BRDF_PBR(float3 N, float3 V, float3 L, float3 albedo, float metallic, float roughness)
{
    float3 H = normalize(V + L);

    float NdotL = saturate(dot(N, L));
    float NdotV = saturate(dot(N, V));

    // Fresnel
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);
    float3 F = FresnelSchlick(saturate(dot(H, V)), F0);

    // Distribution & Geometry
    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);

    // Cook-Torrance Specular
    float3 numerator = D * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.001;
    float3 specular = numerator / denominator;

    // Diffuse (Lambert + energy conservation)
    float3 kd = (1.0 - F) * (1.0 - metallic);
    float3 diffuse = kd * albedo / PI;

    return (diffuse + specular) * NdotL;
}
// 물리 기반 조명(PBR) 계산을 위한 감쇠 및 각도 계산 함수들
float CalcPointAttenuation(float distance, float rangeRcp)
{
    // 거리 기반 감쇠 (Standard Clamped Distance Attenuation)
    // rangeRcp는 1.0 / range 입니다. (곱셈이 나눗셈보다 빠름)
    float distanceNorm = distance * rangeRcp;
    float attenuation = saturate(1.0f - (distanceNorm * distanceNorm));
    return attenuation * attenuation; // 부드러운 감쇠 곡선
}

float CalcSpotCone(float3 lightDir, float3 spotDir, float cosInnerAngle, float cosOuterAngle)
{
    // 스팟라이트 원뿔 각도 계산
    float cosTheta = dot(-lightDir, spotDir);
    // 내각과 외각 사이를 부드럽게 보간 (Smoothstep 유사 기능)
    return saturate((cosTheta - cosOuterAngle) / (cosInnerAngle - cosOuterAngle));
}

float4 MainPS(VSToPS input) : SV_Target
{
    int3 location3 = int3(input.position.xy, 0);
    float3 albedo = GBuffer_AlbedoRoughness.Load(location3).xyz;
    float roughness = GBuffer_AlbedoRoughness.Load(location3).w;
    float3 normal = GBuffer_NormalMetallic.Load(location3).xyz;
    normal = normal * 2.0f - 1.0f;
    float metallic = GBuffer_NormalMetallic.Load(location3).w;
    uint objectID = GBuffer_ObjectID.Load(location3);
    float depth = GBuffer_Depth.Load(location3).r;
   
    float2 ndc = input.cpPos;
    float4 rawWorldPos = mul(float4(ndc, depth, 1.0f), frameData.inverseViewProjection);
    float3 worldPos = rawWorldPos.xyz / rawWorldPos.w;

    float3 V = normalize(frameData.cameraPosition.xyz - worldPos);
    
    // 1. 기본 앰비언트(환경광) 계산
    float3 ambientColor = weatherData.ambientColor.xyz;
    float3 ambient = albedo * ambientColor;

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
            float3 lightToPixel = light.position - worldPos;
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
            float3 lightToPixel = light.position - worldPos;
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
        float3 radiance = BRDF_PBR(normal, V, lightDir, albedo, metallic, roughness) * light.color * light.intensity * attenuation;
        totalPbrLighting += radiance;
    }

    // 3. 최종 색상 조합 (앰비언트 + 모든 라이트의 합)
    float3 finalColor = ambient + totalPbrLighting;
    
    // 계층구조일 땐 방법을 달리해야한다.
    if (objectID != 0 && objectID == selectedObject.id)
    {
        finalColor = lerp(finalColor, float3(1.0f, 0.8f, 0.2f), 0.3f);
    }
    
    // 감마 보정
    finalColor = pow(finalColor, 1.0 / 2.2);
    return float4(finalColor, 1.0f);
}

/*
float4 MainPS(VSToPS input) : SV_Target
{
    int3 location3 = int3(input.position.xy, 0);
    float3 albedo = GBuffer_AlbedoRoughness.Load(location3).xyz;
    float roughness = GBuffer_AlbedoRoughness.Load(location3).w;
    float3 normal = GBuffer_NormalMetallic.Load(location3).xyz;
    normal = normal * 2.0f - 1.0f;
    float metallic = GBuffer_NormalMetallic.Load(location3).w;
    uint objectID = GBuffer_ObjectID.Load(location3);
    float depth = GBuffer_Depth.Load(location3).r;
   
    float2 ndc = input.cpPos;
    float4 rawWorldPos = mul(float4(ndc, depth, 1.0f), frameData.inverseViewProjection);
    float3 worldPos = rawWorldPos.xyz / rawWorldPos.w;

    float3 V = normalize(frameData.cameraPosition.xyz - worldPos);
    
    float3 ambientColor = weatherData.ambientColor.xyz;
    float3 lightColor = weatherData.lightColor.xyz;
    float3 lightDir = -weatherData.lightDir.xyz;

    float3 ambient = albedo * ambientColor;
    float3 pbrLighting = BRDF_PBR(normal, V, lightDir, albedo, metallic, roughness) * lightColor;

    float3 finalColor = ambient + pbrLighting;
    
    // 계층구조일 땐 방법을 달리해야한다.
    if (objectID != 0 && objectID == selectedObject.id)
    {
        finalColor = lerp(finalColor, float3(1.0f, 0.8f, 0.2f), 0.3f);
    }
    
    finalColor = pow(finalColor, 1.0 / 2.2);
    return float4(finalColor, 1.0f);
}
*/