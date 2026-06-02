#include "Common.hlsl"

struct VS_INPUT
{
    float4 position : POSITION0;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 tangent : TANGENT0;
    float3 bitangent : BINORMAL0;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 normal : NORMAL0;
    float3 tangent : TANGENT0;
    float3 bitangent : BINORMAL0;
};

VS_OUTPUT MainVS(VS_INPUT input)
{
    VS_OUTPUT output;
    
    input.position.w = 1.0f;
    output.position = mul(input.position, cbModelVertex.world);
    output.worldPos = output.position.xyz;
    output.position = mul(output.position, mul(cbCameraVertex.view, cbCameraVertex.projection));
    output.tex = input.tex;
    output.normal = mul(input.normal, (float3x3) cbModelVertex.world);
    output.normal = normalize(output.normal);
    output.tangent = mul(input.tangent, (float3x3) cbModelVertex.world);
    output.tangent = normalize(output.tangent);
    output.bitangent = mul(input.bitangent, (float3x3) cbModelVertex.world);
    output.bitangent = normalize(output.bitangent);
    
    return output;
}

float3 CalcuDirLight(float3 worldPos, float3 normal, float3 diffColor)
{
    float3 finalColor;
    
    // Ambient
    float3 ambientColor = float3(0.05f, 0.05f, 0.05f);
    finalColor = ambientColor;
    
    // Phong diffuse
    float NDotL = saturate(dot(-cbLightPixel.direction, normal));
    if (NDotL > 0.0f)
    {
        finalColor += cbLightPixel.color * NDotL;
    }
    
	// Blinn specular
    float3 toEye = cbCameraPixel.position - worldPos;
    toEye = normalize(toEye);
    float3 halfWay = normalize(toEye + -cbLightPixel.direction);
    float NDotH = saturate(dot(halfWay, normal));
    
    finalColor += cbLightPixel.color *
    pow(NDotH, 250.0f) * 0.25f;
    
    return diffColor * finalColor;
}

float3 CalcuPointLight(float3 worldPos, float3 normal, float3 diffColor)
{
    float3 toLight = cbLightPixel.position - worldPos;
    float3 toEye = cbCameraPixel.position - worldPos;
    float distance = length(toLight);
    
    // phong diffuse
    toLight /= distance;
    float NDotL = saturate(dot(toLight, normal));
    float3 finalColor = cbLightPixel.color * NDotL;
    
    // blinn specular
    toEye = normalize(toEye);
    float3 halfWay = normalize(toEye + toLight);
    float NDotH = saturate(dot(halfWay, normal));
    finalColor += cbLightPixel.color * pow(NDotH, 250.0f) * 0.25f;
    
    // attenuation
    float distToLightNorm = 1.0f - saturate(distance * cbLightPixel.rangeRcp);
    float attn = distToLightNorm * distToLightNorm;
    
    finalColor *= diffColor * attn;
    
    return finalColor;
}

float SpotShadowPCF(float3 position)
{
    float4 shadowMapPos = mul(float4(position, 1.0f), cbLightPixel.shadow);
    float3 uvd = shadowMapPos.xyz / shadowMapPos.w;
    uvd.xy = 0.5f * uvd.xy + 0.5f;
    uvd.y = 1.0f - uvd.y;
    
    return SpotShadowMap.SampleCmpLevelZero(PcfSampler, uvd.xy, uvd.z);
}

float3 CalcuSpotLight(float3 worldPos, float3 normal, float3 diffColor)
{
    float3 toLight = cbLightPixel.position - worldPos;
    float3 toEye = cbCameraPixel.position - worldPos;
    float distance = length(toLight);
    
    // phong diffuse
    toLight /= distance;
    float NDotL = saturate(dot(toLight, normal));
    float3 finalColor = cbLightPixel.color * NDotL;
    
    // blinn specular
    toEye = normalize(toEye);
    float3 halfWay = normalize(toEye + toLight);
    float NDotH = saturate(dot(halfWay, normal));
    finalColor += cbLightPixel.color * pow(NDotH, 250.0f) * 0.25f;
    
    // attenuation
    float distToLightNorm = 1.0f - saturate(distance * cbLightPixel.rangeRcp);
    float distAttn = distToLightNorm * distToLightNorm;
    
    // cone attenuation
    float cosAng = acos(dot(-cbLightPixel.direction, toLight));
    float coneAtt = 1.0f * smoothstep(cbLightPixel.outerConeAngle, cbLightPixel.innerConeAngle, cosAng);
    
    float shadowAtt = 1.0f;
    if(cbLightPixel.shadowEnabled)
        shadowAtt = SpotShadowPCF(worldPos);
    
    finalColor *= diffColor * distAttn * coneAtt * shadowAtt;
    
    return finalColor;
}

float4 MainPS(VS_OUTPUT input) : SV_TARGET
{
    float4 diffColor;
    if (!HasDiffuseTexture())
    {
        diffColor = cbModelPixel.color;
    }
    else
    {
        diffColor = DiffuseMap.Sample(LinearSampler, input.tex);
        //diffColor = float4(diffColor.rgb * diffColor.rgb, diffColor.a);
    }

    float3 normal = input.normal;
    if (HasNormalTexture())
    {
        float4 bumpMap = NormalMap.Sample(LinearSampler, input.tex);
        bumpMap = (bumpMap * 2.0f) - 1.0f;

        normal = normalize((bumpMap.x * input.tangent) + (bumpMap.y * input.bitangent) + (bumpMap.z * input.normal));
    }

    float3 lightColor;
    if (IsDirectionalLight())
        lightColor = CalcuDirLight(input.worldPos, normal, diffColor.rgb);
    if (IsPointLight())
        lightColor = CalcuPointLight(input.worldPos, normal, diffColor.rgb);
    if (IsSpotLight())
        lightColor = CalcuSpotLight(input.worldPos, normal, diffColor.rgb);
    
    //return float4(sqrt(lightColor.rgb), diffColor.a);
    return float4(lightColor.rgb, diffColor.a);
}