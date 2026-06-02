// 추후 파일이름을 ConstantBuffer로 바꾸자

cbuffer cbCameraVertex : register(b0)
{
    matrix CameraView : packoffset(c0);
    matrix CameraProj : packoffset(c4);
};

cbuffer cbCameraDomain : register(b0)
{
    matrix CameraViewProj : packoffset(c0);
};

cbuffer cbCameraPixel : register(b0)
{
    float4 CameraPerspectiveValue   : packoffset(c0);
    matrix CameraViewInverse        : packoffset(c1);
};

#define CameraPos (CameraViewInverse[3].xyz)

cbuffer cbObjectVertex : register(b1)
{
    matrix ObjectWorld : packoffset(c0);
};

cbuffer cbMaterialPixel : register(b5)  // 왜 b1은 에러가 날까?
{
    float4 MaterialDiff         : packoffset(c0);
    float4 MaterialNormal       : packoffset(c1);
    float2 MaterialTexTiling    : packoffset(c2);
    float2 MaterialOffset       : packoffset(c2.z);
    uint MaterialProperties     : packoffset(c3);
};

//======================================================================================

struct VSConstBuf_Model
{
    matrix world;
};

struct VSConstBuf_Camera
{
    matrix view;
    matrix projection;
};

struct VSConstBuf_Light
{
    matrix shadow;
};

struct DSConstBuf_Light
{
    matrix lightProjection;
};

struct PSConstBuf_Model
{
    float4 color;
    float4 normal;
    
    float2 tiling;
    float2 offset;
    
    uint properties;
    float3 padding;
};

struct PSConstBuf_Camera
{
    float3 position;
    float padding;
    
    float4 perspectiveValue;
    
    matrix viewInverse;
};

struct PSConstBuf_Light
{
    float3 color;
    float outerConeAngle;
    
    float3 position;
    float rangeRcp;
    
    float3 direction;
    float innerConeAngle;
    
    uint options;
    int shadowEnabled;
    float2 padding;
    
    matrix shadow;
};

// VS ConstantBuffers
cbuffer VS_Model : register(b0)
{ 
    VSConstBuf_Model cbModelVertex;
}

cbuffer VS_Camera : register(b1)
{
    VSConstBuf_Camera cbCameraVertex;
}

cbuffer VS_Light : register(b2)
{
    VSConstBuf_Light cbLightVertex;
}

// DS Constantbuffer
cbuffer DS_Light : register(b0)
{
    DSConstBuf_Light cbLightDomain;
}

// PS Constantuffers
cbuffer PS_Model : register(b0)
{
    PSConstBuf_Model cbModelPixel;
}

cbuffer PS_Camera : register(b1)
{
    PSConstBuf_Camera cbCameraPixel;
}

cbuffer PS_Light : register(b3)     // 임시
{
    PSConstBuf_Light cbLightPixel;
}

// cbLightPixel options
bool IsDirectionalLight() 
{ 
    return cbLightPixel.options & uint(1U << 0); 
}

bool IsPointLight()
{
    return cbLightPixel.options & uint(1U << 1);
}

bool IsSpotLight()
{
    return cbLightPixel.options & uint(1U << 2);
}

// cbModelPixel options
bool HasDiffuseTexture()
{
    //return cbModelPixel.properties & uint(1U << 0);
    return MaterialProperties & uint(1U << 0);
}

bool HasNormalTexture()
{
    //return cbModelPixel.properties & uint(1U << 1);
    return MaterialProperties & uint(1U << 1);
}