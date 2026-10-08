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
    
    float3 emissiveFactor;
    float roughnessFactor;
    
    float2 tiling;
    float2 offset;
    
    float metallicFactor;
    uint flags;
    
    float heightScale;
    uint padding;
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
    float3 color;
    uint type;
    
    float3 position;
    float rangeRcp;
    
    float3 direction;
    float intensity;
    
    float cosInnerAngle;
    float cosOuterAngle;
    float2 paddingRow4;
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