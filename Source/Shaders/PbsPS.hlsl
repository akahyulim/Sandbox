#include "Constants.hlsli"
#include "Resources.hlsli"

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
    // Albedo
    float4 albedo = HasDiffuseMap() ?
        DiffuseMap.Sample(WrapLinearSampler, input.UV) * materialData.diffuseColor : materialData.diffuseColor;

    // Metallic & Roughness
    float metallic = 1.0;//HasMetallicMap() ?
        //MetallicMap.Sample(WrapLinearSampler, input.UV).r : materialData.metallic;
    float roughness = 0.2;//HasRoughnessMap() ?
        //RoughnessMap.Sample(WrapLinearSampler, input.UV).r : materialData.roughness;

    // Normal
    float3 normal = input.Normal;
    if (HasNormalMap())
    {
        float3 bumpMap = NormalMap.Sample(WrapLinearSampler, input.UV).xyz * 2.0f - 1.0f;
        float3x3 TBN = float3x3(input.Tangent, input.BiNormal, input.Normal);
        normal = normalize(mul(bumpMap, TBN));
    }

    float3 V = normalize(frameData.position.xyz - input.WorldPos);

/*
    float3 finalColor = cbForwardLightPS.ambientColor;
    for (int i = 0; i < cbForwardLightPS.lightCount; ++i)
    {
        Light light = cbForwardLightPS.lights[i];
        float3 L;

        if (light.type == 0) // Directional
            L = normalize(-light.direction);
        else
            L = normalize(light.position - input.WorldPos);

        finalColor += BRDF_PBR(normal, V, L, albedo.xyz, metallic, roughness) * light.color * light.intensity;
    }
*/
    float3 finalColor = float3(0.1, 0.1, 0.1);
    LightData light = lightData;
    float3 L = normalize(-light.direction);
    finalColor += BRDF_PBR(normal, V, L, albedo.xyz, metallic, roughness) * light.color;
    
    return float4(finalColor, albedo.a);//1.0f);
}
