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
    
        albedoColor *= texColor.xyz;
        albedoColor *= albedoColor;
    
        finalAlpha *= texColor.w;
    }
    
    float roughness = materialData.roughnessFactor;
    float metallic = materialData.metallicFactor;

    if (HasORMMap())
    {
        float3 orm = ORMMap.Sample(WrapLinearSampler, uv).xyz;
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
    
    float3 ambientColor = weatherData.ambientColor.xyz;
    float3 ambient = albedoColor * ambientColor;
    
    float3 sunLightColor = weatherData.lightColor.xyz;
    float3 sunLightDir = normalize(-weatherData.lightDir);
    float sunIntensity = 3.0f;
    
    float3 totalPbrLighting = BRDF_PBR(normal, V, sunLightDir, albedoColor, metallic, roughness)
                            * sunLightColor * sunIntensity * 1.0f;

    for (uint i = 0; i < lights.lightCount; ++i)
    {
        LightData light = lights.lights[i];
        
        float3 lightDir = 0.0f;
        float attenuation = 1.0f;
        
        if (light.type == 0)
        {
            float3 lightToPixel = light.position - input.WorldPos;
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
            float3 lightToPixel = light.position - input.WorldPos;
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
        
        float3 radiance = BRDF_PBR(normal, V, lightDir, albedoColor, metallic, roughness)
                       * light.color * light.intensity * attenuation;
                       
        totalPbrLighting += radiance;
    }

    float3 finalColor = ambient + totalPbrLighting;
   
    // 계층구조일 땐 방법을 달리해야한다.
    if (objectData.id != 0 && objectData.id == selectedObject.id)
    {
        finalColor = lerp(finalColor, float3(1.0f, 0.8f, 0.2f), 0.3f);
    }
    
    finalColor = pow(finalColor, 1.0 / 2.2);
    
    PSOutput output;
    output.ObjectID = objectData.id;
    output.ldrRtv = float4(finalColor, finalAlpha);
    
    return output;
}