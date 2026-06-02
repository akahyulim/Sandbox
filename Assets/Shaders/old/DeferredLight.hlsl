#include "Common.hlsl"

// 이건 클립 스페이스다.( -1.0 ~ 1.0 )
static const float2 arrBasePos[4] =
{
    float2(-1.0, 1.0),  // 좌상
	float2(1.0, 1.0),   // 우상
	float2(-1.0, -1.0), // 좌하
	float2(1.0, -1.0),  // 우하
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 cpPos : TEXCOORD0;
};

// 책과 달리 0 ~ 3이 맞는 듯 보인다.
VS_OUTPUT MainVS(uint VertexID : SV_VERTEXID)
{
    VS_OUTPUT output;

    output.position = float4(arrBasePos[VertexID].xy, 0.0, 1.0);
    output.cpPos = output.position.xy;

    return output;
}

// 우선은 디렉셔널 라이트가 무조건 첫 번째 광원이라는 가정하에 수행한다.
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
    
    finalColor += cbLightPixel.color * pow(NDotH, 250.0f) * 0.25f;
    
    return diffColor * finalColor;
}

float PointShadowPCF(float3 ToPixel)
{
    float3 ToPixelAbs = abs(ToPixel);
    float Z = max(ToPixelAbs.x, max(ToPixelAbs.y, ToPixelAbs.z));
    //float Depth = (LightPerspectiveValues.x * Z + LightPerspectiveValues.y) / Z;
    return 1.0f; //PointShadowMapTexture.SampleCmpLevelZero(PcfSampler, ToPixel, Depth);
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
    
    // shadow
    float shadowAtt = 1.0f;
    if (cbLightPixel.shadowEnabled)
        shadowAtt = PointShadowPCF(worldPos - cbLightPixel.position);
    
    finalColor *= diffColor * attn * shadowAtt;
    
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
    
    // shadow
    float shadowAtt = 1.0f;
    if (cbLightPixel.shadowEnabled)
        shadowAtt = SpotShadowPCF(worldPos);
    
    finalColor *= diffColor * distAttn * coneAtt * shadowAtt;
    
    return finalColor;
}

float4 MainPS(VS_OUTPUT input) : SV_TARGET
{
	// Diff Color
    int3 location3 = int3(input.position.xy, 0);
    float4 diffColor = DiffuseTex.Load(location3);
    
	// Linear Depth
    float depth = DepthTex.Load(location3).x;
    float linearDepth = cbCameraPixel.perspectiveValue.z / (depth + cbCameraPixel.perspectiveValue.w);

	// World Position
    float4 position;
    position.xy = input.cpPos.xy * cbCameraPixel.perspectiveValue.xy * linearDepth;
    position.z = linearDepth;
    position.w = 1.0f;
    position = mul(position, cbCameraPixel.viewInverse);

	// Normal
    float3 normal;
    normal = NormalTex.Load(location3);
    normal = normalize(normal * 2.0f - 1.0f);
    
    float3 lightColor;
    if (IsDirectionalLight())
        lightColor = CalcuDirLight(position.xyz, normal, diffColor.rgb);
    if (IsPointLight())
        lightColor = CalcuPointLight(position.xyz, normal, diffColor.rgb);
    if (IsSpotLight())
        lightColor = CalcuSpotLight(position.xyz, normal, diffColor.rgb);
    
    return float4(lightColor.xyz, 1.0f);
}