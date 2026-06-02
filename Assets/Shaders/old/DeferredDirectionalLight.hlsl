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

cbuffer cbPixel : register(b2)
{
    float3 LightColor   : packoffset(c0);
    float3 ToLight      : packoffset(c1);
    uint ShadowEnabled  : packoffset(c1.w);  
    float4x4 ToShadowSpace : packoffset(c2);
    float4 ToCascadeOffsetX : packoffset(c6);
    float4 ToCascadeOffsetY : packoffset(c7);
    float4 ToCascadeScale : packoffset(c8);
};

float CascadedShadow(float3 position)
{
	// Transform the world position to shadow space
    float4 posShadowSpace = mul(float4(position, 1.0), ToShadowSpace);

	// Transform the shadow space position into each cascade position
    float4 posCascadeSpaceX = (ToCascadeOffsetX + posShadowSpace.xxxx) * ToCascadeScale;
    float4 posCascadeSpaceY = (ToCascadeOffsetY + posShadowSpace.yyyy) * ToCascadeScale;

	// Check which cascade we are in
    float4 inCascadeX = abs(posCascadeSpaceX) <= 1.0;
    float4 inCascadeY = abs(posCascadeSpaceY) <= 1.0;
    float4 inCascade = inCascadeX * inCascadeY;

	// Prepare a mask for the highest quality cascade the position is in
    float4 bestCascadeMask = inCascade;
    bestCascadeMask.yzw = (1.0 - bestCascadeMask.x) * bestCascadeMask.yzw;
    bestCascadeMask.zw = (1.0 - bestCascadeMask.y) * bestCascadeMask.zw;
    bestCascadeMask.w = (1.0 - bestCascadeMask.z) * bestCascadeMask.w;
    float bestCascade = dot(bestCascadeMask, float4(0.0, 1.0, 2.0, 3.0));

	// Pick the position in the selected cascade
    float3 UVD;
    UVD.x = dot(posCascadeSpaceX, bestCascadeMask);
    UVD.y = dot(posCascadeSpaceY, bestCascadeMask);
    UVD.z = posShadowSpace.z;

	// Convert to shadow map UV values
    UVD.xy = 0.5 * UVD.xy + 0.5;
    UVD.y = 1.0 - UVD.y;

	// Compute the hardware PCF value
	// 텍스쳐 배열이므로 매개변수가 달라진다. 
	// 즉, 두 번째 매개변수에서 배열의 인덱스를 받는다.
    float shadow = CascadeShadowMap.SampleCmpLevelZero(PcfSampler, float3(UVD.xy, bestCascade), UVD.z);
	
	// set the shadow to one (fully lit) for positions with no cascade coverage
    shadow = saturate(shadow + 1.0 - any(bestCascadeMask));
	
    return shadow;
}

// 마지막에 그림자 적용 여부를 전달받도록 하자.
float3 CalcuDirLight(float3 worldPos, float3 normal, float3 diffColor, bool shadow)
{
    float3 finalColor;
    
    // Ambient
    float3 ambientColor = float3(0.05f, 0.05f, 0.05f);
    finalColor = ambientColor;
    
    // Phong diffuse
    float NDotL = saturate(dot(ToLight, normal));
    finalColor += LightColor * NDotL;
    
	// Blinn specular
    float3 toCamera = CameraPos - worldPos;
    toCamera = normalize(toCamera);
    float3 halfWay = normalize(toCamera + ToLight);
    float NDotH = saturate(dot(halfWay, normal));
    
    finalColor += pow(NDotH, 250.0) * 0.25;
    
     // 그림자 감쇠
    float shadowAtt = 1.0;
    if (ShadowEnabled == 1)   
        shadowAtt = CascadedShadow(worldPos);
    
    return finalColor * diffColor * shadowAtt;
}

float4 MainPS(VS_OUTPUT input) : SV_TARGET0
{
	// Diff LightColor
    int3 location3 = int3(input.position.xy, 0);
    float4 diffColor = DiffuseTex.Load(location3);
    
	// Linear Depth
    float depth = DepthTex.Load(location3).x;
    float linearDepth = CameraPerspectiveValue.z / (depth + CameraPerspectiveValue.w);
    
	// World Position
    float4 position;
    position.xy = input.cpPos.xy * CameraPerspectiveValue.xy * linearDepth;
    position.z = linearDepth;
    position.w = 1.0f;
    position.xyz = mul(position, CameraViewInverse).xyz;

	// Normal
    float3 normal;
    normal = NormalTex.Load(location3);
    normal = normalize(normal * 2.0 - 1.0);
    
    // 추후 상수버퍼를 통해 그림자 적용 여부를 전달받아야 한다.
    float3 lightColor = CalcuDirLight(position.xyz, normal, diffColor.rgb, false);
    
    return float4(lightColor.xyz, 1.0);
}