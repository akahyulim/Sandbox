
struct VSInput_Unlit
{
    float4 Position : POSITION;
    float2 UV : TEXCOORD0;
};

struct VSInput_Lit
{
    float4 Position : POSITION;
    float2 UV : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 BiNormal : BINORMAL;
};