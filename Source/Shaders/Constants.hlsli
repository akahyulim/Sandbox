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
    float4 baseColor; // [16바이트] 알베도 색상 (RGBA)
    
    float3 emissiveFactor; // [12바이트] 자체 발광 색상
    float roughnessFactor; // [ 4바이트] 거칠기 (합쳐서 16바이트)
    
    float2 tiling; // [ 8바이트] UV 타일링
    float2 offset; // [ 8바이트] UV 오프셋 (합쳐서 16바이트)
    
    float metallicFactor; // [ 4바이트] 금속성
    uint flags; // [ 4바이트] 텍스처 유무 플래그 등
    
    float heightScale;
    uint padding; // [ 8바이트] 16바이트 배수를 맞추기 위한 패딩 (합쳐서 16바이트)
};

struct WeatherData
{
    float4 lightDir;
    float4 lightColor;
    float4 ambientColor;
    float4 skyColor;
};

struct LightData
{
    float3 color; // c0.xyz
    uint type; // c0.w  (0: Directional, 1: Point, 2: Spot)
    
    float3 position; // c1.xyz
    float rangeRcp; // c1.w
    
    float3 direction; // c2.xyz
    float intensity; // c2.w
    
    float cosInnerAngle; // c3.x
    float cosOuterAngle; // c3.y
    float2 paddingRow4; // c3.zw
};

struct LightConstants
{
    LightData lights[32];
    uint lightCount;
    uint padding[3];
};

struct SelectedObjectData
{
    uint id;
    uint3 dummy;
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

cbuffer cbWeather : register(b3)
{
    WeatherData weatherData;
}

cbuffer cbLightData : register(b4)
{
    LightConstants lights;
}

cbuffer cbSelectedObjectData : register(b5)
{
    SelectedObjectData selectedObject;
}

bool HasAlbedoMap()
{
    return materialData.flags & (1 << 0);
}
bool HasNormalMap()
{
    return materialData.flags & (1 << 1);
}

bool HasORMMap()
{
    return materialData.flags & (1 << 2);
}

bool HasDisplacementMap()
{
    return materialData.flags & (1 << 3);
}

bool HasRoughnessMap()
{
    return materialData.flags & (1 << 4);
}

bool HasMetallicMap()
{
    return materialData.flags & (1 << 5);
}