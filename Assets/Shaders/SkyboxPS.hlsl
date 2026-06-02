#include "Resources.hlsli"

struct PSInput
{
    float4 position : SV_Position;
    float3 direction : TEXCOORD;
};

float4 MainPS(PSInput input) : SV_Target
{
   return Skymap.Sample(SkyboxSampler, normalize(input.direction));
}
