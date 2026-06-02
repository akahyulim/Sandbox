cbuffer MatrixBuffer
{
    matrix modelMatrix;
    matrix viewMatrix;
    matrix projMatrix;
};

struct VS_INPUT
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 tex : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL0;
    float3 worldPos : TEXCOORD1;
    float3 tangent : TANGENT0;
    float2 tex : TEXCOORD0;
};

VS_OUTPUT MainVS(VS_INPUT input)
{
    VS_OUTPUT output;

    float4 position = input.position;
    position.w = 1.0f;

    float4 worldPos = mul(position, modelMatrix);
    output.worldPos = worldPos.xyz;

    output.position = mul(worldPos, viewMatrix);
    output.position = mul(output.position, projMatrix);

    output.tex = input.tex;

    float3x3 normalMatrix = (float3x3) modelMatrix;
    output.normal = normalize(mul(input.normal, normalMatrix));
    output.tangent = normalize(mul(input.tangent, normalMatrix));

    return output;
}

cbuffer MaterialBuffer
{
    float4 diffuseColor;
    float2 tiling;
    float2 offset;
};

// ─────────────────────────────
// Material Textures (t0 ~ t5)
// ─────────────────────────────
Texture2D DiffuseMap : register(t0);
Texture2D NormalMap : register(t1);
Texture2D RoughnessMap : register(t2);
Texture2D MetallicMap : register(t3);
Texture2D EmissiveMap : register(t4);
Texture2D OpacityMap : register(t5);

// ─────────────────────────────
// G-Buffer Inputs (t10 ~ t15)
// ─────────────────────────────
Texture2D<float4> GBuffer_Albedo : register(t10);
Texture2D<float3> GBuffer_Normal : register(t11);
Texture2D<float2> GBuffer_RoughMetal : register(t12);
Texture2D<float4> GBuffer_Emissive : register(t13);
Texture2D<float> GBuffer_Depth : register(t14);
Texture2D<uint> GBuffer_MaterialID : register(t15);

// ─────────────────────────────
// Shadow & Lighting Maps (t20 ~ t25)
// ─────────────────────────────
Texture2D<float> ShadowMap : register(t20);
Texture2D<float> SSAOMap : register(t21);
TextureCube<float4> EnvironmentMap : register(t22);
TextureCube<float3> IBL_Irradiance : register(t23);
TextureCube<float3> IBL_Specular : register(t24);
Texture2D<float2> BRDF_LUT : register(t25);

// ─────────────────────────────
// Post-Processing Inputs (t30 ~ t33)
// ─────────────────────────────
Texture2D<float4> SceneColor : register(t30);
Texture2D<float> SceneDepth : register(t31);
Texture2D<float4> BloomTexture : register(t32);
Texture2D<float2> MotionVector : register(t33);

float4 MainPS(VS_OUTPUT input) : SV_TARGET
{
    return diffuseColor;
}