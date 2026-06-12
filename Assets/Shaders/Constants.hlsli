#define MAX_LIGHTS 16

// ==================================================================
// [b0] Global / Camera Buffer (업데이트 빈도: 프레임당 1번)
// ==================================================================
struct CameraData
{
    matrix viewMatrix;
    matrix projMatrix;
    matrix viewProjMatrix;
    float4 position;
    float4 backgroundColor;
};

cbuffer cbCamera : register(b0)
{
    CameraData gCamera; // 🌟 gCameraData보다 짧고 직관적이라 유지
}

// ==================================================================
// [b1] Material Buffer (업데이트 빈도: 마티리얼 변경 시)
// ==================================================================
struct MaterialData
{
    float4 diffuseColor;
    float2 tiling;
    float2 offset;
    
    uint flags;
    uint renderingMode;
    uint2 padding; 
};

cbuffer cbMaterial : register(b1)
{
    MaterialData gMaterial;
}

bool HasDiffuseMap()  { return gMaterial.flags & (1 << 0); }
bool HasNormalMap()   { return gMaterial.flags & (1 << 1); }

// ==================================================================
// [b2] Object Buffer (업데이트 빈도: 오브젝트를 그릴 때마다)
// ==================================================================
struct ObjectData
{
    matrix worldMatrix;
};

cbuffer cbObject : register(b2) 
{
    ObjectData gObject;
}

// ==================================================================
// [b3] Lighting Buffer (업데이트 빈도: 씬/패스당 1번 혹은 라이트 변경 시)
// ==================================================================
struct LightData
{
    float3 color;          // c0.xyz
    uint   type;           // c0.w  (0: Directional, 1: Point, 2: Spot)
    
    float3 position;       // c1.xyz
    float  rangeRcp;       // c1.w
    
    float3 direction;      // c2.xyz
    float  paddingRow3;    // c2.w
    
    float  cosInnerAngle;  // c3.x
    float  cosOuterAngle;  // c3.y
    float2 paddingRow4;    // c3.zw
};

cbuffer cbLight : register(b3) // 🌟 버퍼 이름도 cbLightBuffer에서 cbLight로 통일!
{
    LightData gLight;
}