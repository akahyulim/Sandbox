
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