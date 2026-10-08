#include "Constants.hlsli"
#include "Resources.hlsli"
#include "Lighting.hlsli"

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
    float3 ambient = albedo * ambientColor;
    
    float3 sunLightColor = weatherData.lightColor.xyz;
    float3 sunLightDir = normalize(-weatherData.lightDir);
    float sunIntensity = 3.0f;
    
    float3 totalPbrLighting = BRDF_PBR(normal, V, sunLightDir, albedo, metallic, roughness)
                            * sunLightColor * sunIntensity * 1.0f;

    for (uint i = 0; i < lights.lightCount; ++i)
    {
        LightData light = lights.lights[i];
        
        float3 lightDir = 0.0f;
        float attenuation = 1.0f;
        
        if (light.type == 0)
        {
            float3 lightToPixel = light.position - worldPos;
            float distance = length(lightToPixel);
            
            float range = 1.0f / light.rangeRcp;
            if (distance > range) 
                continue;
                
            lightDir = lightToPixel / distance;
            
            float distanceNorm = distance * light.rangeRcp;
            float atten = saturate(1.0f - (distanceNorm * distanceNorm));
            attenuation = atten * atten;
        }
        else if (light.type == 1)
        {
            float3 lightToPixel = light.position - worldPos;
            float distance = length(lightToPixel);
            
            float range = 1.0f / light.rangeRcp;
            if (distance > range) 
                continue;
                
            lightDir = lightToPixel / distance;
            
            float distanceNorm = distance * light.rangeRcp;
            float pointAtten = saturate(1.0f - (distanceNorm * distanceNorm));
            pointAtten = pointAtten * pointAtten;
            
            float cosTheta = dot(-lightDir, light.direction);
            float spotAtten = saturate((cosTheta - light.cosOuterAngle) / (light.cosInnerAngle - light.cosOuterAngle));
            
            attenuation = pointAtten * spotAtten;
        }
        else
        {
            continue;
        }
        
        float3 radiance = BRDF_PBR(normal, V, lightDir, albedo, metallic, roughness)
                       * light.color * light.intensity * attenuation;
                       
        totalPbrLighting += radiance;
    }

    float3 finalColor = ambient + totalPbrLighting;
    
    // 계층구조일 땐 방법을 달리해야한다.
    if (objectID != 0 && objectID == selectedObject.id)
    {
        finalColor = lerp(finalColor, float3(1.0f, 0.8f, 0.2f), 0.3f);
    }
    
    finalColor = pow(finalColor, 1.0 / 2.2);
    
    return float4(finalColor, 1.0f);
}