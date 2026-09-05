#include "Constants.hlsli"
#include "Resources.hlsli"

struct PickingData
{
    float4 position;
    float4 normal;
    uint id;
    uint3 pad;
};

RWStructuredBuffer<PickingData> PickingBuffer : register(u0);

[numthreads(1, 1, 1)]
void MainCS()
{
    if (frameData.mousePosition.x < 0 || frameData.mousePosition.x > frameData.screenResolution.x ||
        frameData.mousePosition.y < 0 || frameData.mousePosition.y > frameData.screenResolution.y)
        return;
  
    uint2 mouseCoords = frameData.mousePosition;

    float zw = GBuffer_Depth[mouseCoords].r;

    float2 uv = (mouseCoords + 0.5f) / frameData.screenResolution;
    uv = uv * 2.0f - 1.0f;
    uv.y *= -1.0f;

    float4 worldSpacePosition = mul(float4(uv, zw, 1.0f), frameData.inverseViewProjection);
    float3 viewSpaceNormal = GBuffer_NormalMetallic[mouseCoords].xyz;
    viewSpaceNormal = 2.0f * viewSpaceNormal - 1.0f;
    
    uint objectID = GBuffer_ObjectID.Load(int3(mouseCoords, 0));

    PickingData picking_data;
    picking_data.position = worldSpacePosition / worldSpacePosition.w;
    picking_data.normal = float4(normalize(mul(viewSpaceNormal, (float3x3) transpose(frameData.view))), 0.0f);
    picking_data.id = objectID;
    picking_data.pad = uint3(0, 0, 0);
    
    PickingBuffer[0] = picking_data;
}
