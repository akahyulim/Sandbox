#include "VSInput.hlsli"
#include "Constants.hlsli"

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

VSOutput MainVS(VSInput_Unlit input)
{
    VSOutput output;

    float4 position = input.Position;
    position.w = 1.0f;
    
    output.Position = mul(position, cbObjectVS.worldMatrix);
    output.Position = mul(output.Position, cbCamera.viewProjMatrix);

    output.UV = input.UV;
    
    return output;
}