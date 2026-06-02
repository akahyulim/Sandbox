#include "Constants.hlsli"
#include "Resources.hlsli"

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

float4 MainPS(PSInput input) : SV_TARGET
{
    return HasDiffuseMap() ?
        DiffuseMap.Sample(WrapLinearSampler, input.UV) * cbMaterialPS.diffuseColor : cbMaterialPS.diffuseColor;
}