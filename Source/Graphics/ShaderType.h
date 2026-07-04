#pragma once
#include <DirectXMath.h>

namespace Dive
{
    // 상수 버퍼 내의 배열 크기 지정을 위한 매크로 (HLSL과 동기화)
#define MAX_LIGHTS 16

    // 아무래도 cbXXX보단 XXXData라는 이름이 더 나을 것 같다.
    
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

    struct cbLight
    {
        DirectX::XMFLOAT3 color = { 1.0f, 1.0f, 1.0f };
        uint32_t type = 0;

        DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
        float rangeRcp = 1.0f / 50.0f;

        DirectX::XMFLOAT3 direction = { 0.0f, -1.0f, 0.0f };
        float paddingRow3 = 0.0f;

        float cosInnerAngle = 1.0f;
        float cosOuterAngle = 0.707f;
        uint32_t paddingRow4[2] = { 0, 0 };
    };

    struct cbForwardLight
    {
        DirectX::XMFLOAT4 ambientColor;
        
        int32_t lightCount;
        DirectX::XMFLOAT3 padding;
        
        cbLight lights[MAX_LIGHTS];
    };
}