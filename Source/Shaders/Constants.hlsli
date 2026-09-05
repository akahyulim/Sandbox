#define MAX_LIGHTS 16

struct FrameData
{
    matrix view;
    matrix projection;
    matrix viewProjection;
    matrix inverseViewProjection;
    float4 cameraPosition;
    float4 cameraForward;
    float2 screenResolution;
    float2 mousePosition;
};

struct ObjectData
{
    matrix model;
    uint id;
    uint3 pad;
};

struct MaterialData
{
    float4 baseColor;
    float2 tiling;
    float2 offset;
    
    uint flags;
    uint renderingMode;
    uint2 padding;
};

struct LightData
{
    float3 color; // c0.xyz
    uint type; // c0.w  (0: Directional, 1: Point, 2: Spot)
    
    float3 position; // c1.xyz
    float rangeRcp; // c1.w
    
    float3 direction; // c2.xyz
    float paddingRow3; // c2.w
    
    float cosInnerAngle; // c3.x
    float cosOuterAngle; // c3.y
    float2 paddingRow4; // c3.zw
};

cbuffer cbFrame: register(b0)
{
    FrameData frameData;
}

cbuffer cbObject : register(b1)
{
    ObjectData objectData;
}

cbuffer cbMaterial : register(b2)
{
    MaterialData materialData;
}

cbuffer cbLightData : register(b3)
{
    LightData lightData;
}

bool HasAlbedoMap()
{
    return materialData.flags & (1 << 0);
}
bool HasNormalMap()
{
    return materialData.flags & (1 << 1);
}
