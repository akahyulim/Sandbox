#pragma once
#include <DirectXMath.h>

namespace Dive
{
    // 상수 버퍼 내의 배열 크기 지정을 위한 매크로 (HLSL과 동기화)
#define MAX_LIGHTS 16

// ==================================================================
// [b0] Global / Camera Buffer 구조체
// ==================================================================
    struct CameraData
    {
        DirectX::XMMATRIX viewMatrix;       // 64바이트
        DirectX::XMMATRIX projMatrix;       // 64바이트
        DirectX::XMMATRIX viewProjMatrix;   // 64바이트
        DirectX::XMFLOAT4 position;         // 16바이트 (Vector4)
        DirectX::XMFLOAT4 backgroundColor;  // 16바이트 (Vector4)
    };                                      // 총 224바이트 (16의 배수 OK)

    // ==================================================================
    // [b1] Material Buffer 구조체
    // ==================================================================
    struct MaterialPS
    {
        DirectX::XMFLOAT4 diffuseColor;     // 16바이트
        DirectX::XMFLOAT2 tiling;          // 8바이트
        DirectX::XMFLOAT2 offset;          // 8바이트

        uint32_t          flags;            // 4바이트 (HLSL의 uint)
        uint32_t          renderingMode;    // 4바이트 (HLSL의 uint)
        uint32_t          padding[2];       // 8바이트 (HLSL의 uint2와 매칭되어 16바이트 정렬 완성)
    };                                      // 총 48바이트 (16의 배수 OK)

    // ==================================================================
    // [b2] Object Buffer 구조체
    // ==================================================================
    struct ObjectVS
    {
        DirectX::XMMATRIX worldMatrix;      // 64바이트
    };                                      // 총 64바이트 (16의 배수 OK)

    // ==================================================================
    // [b3] Lighting Buffer 내부 구조체 및 패스 구조체
    // ==================================================================
    struct Light
    {
        uint32_t          type;             // 4바이트
        DirectX::XMFLOAT3 color;            // 12바이트 -> 묶어서 16바이트 (float4 1개)

        float             intensity;        // 4바이트
        DirectX::XMFLOAT3 position;         // 12바이트 -> 묶어서 16바이트 (float4 2개)

        float             rangeRcp;         // 4바이트
        DirectX::XMFLOAT3 direction;        // 12바이트 -> 묶어서 16바이트 (float4 3개)

        float             innerAngle;       // 4바이트
        float             outerAngle;       // 4바이트
        uint32_t          padding[2];       // 8바이트 -> 묶어서 16바이트 (float4 4개)
    };                                      // 개당 64바이트 (16의 배수 OK)

    struct ForwardLightPS
    {
        DirectX::XMFLOAT4 ambientColor;     // 16바이트

        int32_t           lightCount;       // 4바이트 (HLSL의 int)
        DirectX::XMFLOAT3 padding;          // 12바이트 -> 묶어서 16바이트 정렬 완료

        Light             lights[MAX_LIGHTS]; // 64바이트 * 16 = 1024바이트
    };                                      // 총 1056바이트 (16의 배수 OK)
}