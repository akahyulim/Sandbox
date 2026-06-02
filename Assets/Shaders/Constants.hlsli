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

cbuffer CameraBuffer : register(b0)
{
    CameraData cbCamera;
}

// ==================================================================
// [b1] Material Buffer (업데이트 빈도: 마티리얼 변경 시)
// ==================================================================
struct MaterialPS
{
    float4 diffuseColor;
    float2 tiling;
    float2 offset;
    
    uint flags;
    uint renderingMode;
    uint2 padding; // 16바이트 정렬 완벽 (4 + 4 + 8 = 16)
};

cbuffer MaterialBuffer : register(b1)
{
    MaterialPS cbMaterialPS;
}

bool HasDiffuseMap()  { return cbMaterialPS.flags & (1 << 0); }
bool HasNormalMap()   { return cbMaterialPS.flags & (1 << 1); }

// ==================================================================
// [b2] Object Buffer (업데이트 빈도: 오브젝트를 그릴 때마다)
// ==================================================================
struct ObjectVS
{
    matrix worldMatrix;
};

cbuffer ObjectBuffer : register(b2) // 💡 b1에서 b2로 변경하여 독립적인 슬롯 확보!
{
    ObjectVS cbObjectVS;
}

// ==================================================================
// [b3] Lighting Buffer (업데이트 빈도: 씬/패스당 1번 혹은 라이트 변경 시)
// ==================================================================
struct Light
{
    uint type;
    float3 color;        // 💡 type(4) + color(12) = 16바이트 (float4 묶음 1)

    float intensity;
    float3 position;     // 💡 intensity(4) + position(12) = 16바이트 (float4 묶음 2)

    float rangeRcp;
    float3 direction;    // 💡 rangeRcp(4) + direction(12) = 16바이트 (float4 묶음 3)

    float innerAngle;
    float outerAngle;
    float2 padding;      // 💡 angle(4) + angle(4) + padding(8) = 16바이트 (float4 묶음 4)
};

struct ForwardLightPS
{
    float4 ambientColor;
    
    int lightCount;
    float3 padding;      // 💡 lightCount(4) + padding(12) = 16바이트 정렬 완료
    
    Light lights[MAX_LIGHTS];
};

cbuffer ForwardLightBuffer : register(b3) // 💡 b2에서 b3로 변경
{
    ForwardLightPS cbForwardLightPS;
}
/*
#define MAX_LIGHTS 16

// Common ===========================================================
struct Camera
{
    matrix viewMatrix;
    matrix projMatrix;
    matrix viewProjMatrix;
    float4 position;
    float4 backgroundColor;
};

cbuffer CameraBuffer : register(b0)
{
    Camera cbCamera;
}

// Vertex Shader ====================================================
struct ObjectVS
{
    matrix worldMatrix;
};

cbuffer ObjectBuffer : register(b1)
{
    ObjectVS cbObjectVS;
}


// Pixel Shader =====================================================
struct MaterialPS
{
    float4 diffuseColor;
    float2 tiling;
    float2 offset;
    uint flags;
    uint renderingMode;
    uint2 padding;
};

cbuffer MaterialBuffer : register(b1)
{
    MaterialPS cbMaterialPS;
}

bool HasDiffuseMap()  { return cbMaterialPS.flags & (1 << 0); }
bool HasNormalMap()   { return cbMaterialPS.flags & (1 << 1); }

struct Light
{
    uint type;
    float3 color;
    float intensity;
    float3 position;
    float rangeRcp;
    float3 direction;
    float innerAngle;
    float outerAngle;
    float2 padding;
};

struct ForwardLightPS
{
    float4 ambientColor;
    int lightCount;
    float3 padding;
    Light lights[MAX_LIGHTS];
};

cbuffer ForwardLightBuffer : register(b2)
{
    ForwardLightPS cbForwardLightPS;
}
*/