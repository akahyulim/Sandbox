#pragma once
#include <DirectXMath.h>

// Graphics로 옮기라고 한다.
namespace Dive
{
    // 상수 버퍼 내의 배열 크기 지정을 위한 매크로 (HLSL과 동기화)
#define MAX_LIGHTS 16

    // [b0] Global / Camera Buffer 구조체
    struct cbCamera
    {
        DirectX::XMMATRIX viewMatrix;
        DirectX::XMMATRIX projMatrix;
        DirectX::XMMATRIX viewProjMatrix;
        DirectX::XMFLOAT4 position;
        DirectX::XMFLOAT4 backgroundColor;
    };  

    // [b1] Material Buffer 구조체
    struct cbMaterial
    {
        DirectX::XMFLOAT4 diffuseColor;
        DirectX::XMFLOAT2 tiling;
        DirectX::XMFLOAT2 offset;

        uint32_t flags;
        uint32_t renderingMode;
        uint32_t padding[2];
    };

    // [b2] Object Buffer 구조체
    struct cbObject
    {
        DirectX::XMMATRIX worldMatrix;
    };

    // 이건 cbLight라고 보기에 좀 아쉽다.
    // Light이고 아래의 cbForwardLight에 포함되는 데이터다.
    struct cbLight
    {
        uint32_t type;
        DirectX::XMFLOAT3 color;
        
        float intensity;
        DirectX::XMFLOAT3 position;
        
        float rangeRcp;
        DirectX::XMFLOAT3 direction;
        
        float innerAngle;
        float outerAngle;
        uint32_t padding[2];
    };

    struct cbForwardLight
    {
        DirectX::XMFLOAT4 ambientColor;
        
        int32_t lightCount;
        DirectX::XMFLOAT3 padding;
        
        cbLight lights[MAX_LIGHTS];
    };
}