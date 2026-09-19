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