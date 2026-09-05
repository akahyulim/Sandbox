#pragma once
#include <DirectXMath.h>

// adria는 ConstantBuffers.h에 선언해 놓았다.
namespace Dive
{
#define MAX_LIGHTS 16
    
    struct FrameData
    {
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
        DirectX::XMMATRIX viewProjection;
        DirectX::XMMATRIX inverseViewProjection;
        DirectX::XMFLOAT4 cameraPosition;
        DirectX::XMFLOAT4 cameraForward;
        
        DirectX::XMFLOAT2 screenResolution;
        DirectX::XMFLOAT2 mousePosition;
    };  

    struct ObjectData
    {
        DirectX::XMMATRIX model;
        uint32_t id = 0;
        uint32_t pad[3] = {0, 0, 0};
        // transposed_inverse_model???
    };

    struct MaterialData
    {
        DirectX::XMFLOAT4 baseColor;
        DirectX::XMFLOAT2 tiling;
        DirectX::XMFLOAT2 offset;

        uint32_t flags;
        uint32_t renderingMode = 0;
        uint32_t padding[2] = { 0, 0 };
    };

    struct LightData
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

    struct ForwardLightData
    {
        DirectX::XMFLOAT4 ambientColor;
        
        int32_t lightCount;
        DirectX::XMFLOAT3 padding;
        
        LightData lights[MAX_LIGHTS];
    };

    struct PickingData
    {
        DirectX::XMFLOAT4 position = { 0.0f, 0.0f, 0.0f, 1.0f };
        DirectX::XMFLOAT4 normal = { 0.0f, 1.0f, 0.0f, 0.0f };
        uint32_t id = 0;
        DirectX::XMUINT3 pad = { 0, 0, 0 };
    };
}