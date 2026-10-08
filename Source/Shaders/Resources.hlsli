// samplers
SamplerState WrapLinearSampler : register(s0);
SamplerState ClampPointSampler : register(s1);
SamplerState ClampLinearSampler : register(s2);
SamplerState SkyboxSampler : register(s3);
SamplerState SkySphereSampler : register(s4);
SamplerState ShadowCompare : register(s5);

// Material Textures (t0 ~ t5)
Texture2D AlbedoMap : register(t0);
Texture2D NormalMap : register(t1);
Texture2D ORMMap : register(t2);
Texture2D DisplacementMap : register(t3);
Texture2D RoughnessMap : register(t4);
Texture2D MetallicMap : register(t5);

// G-Buffer Inputs (t6 ~ t9)
Texture2D<float4> GBuffer_AlbedoRoughness : register(t6);
Texture2D<float4> GBuffer_NormalMetallic : register(t7);
Texture2D<float4> GBuffer_Emissive : register(t8);
Texture2D<uint> GBuffer_ObjectID : register(t9);
Texture2D<float> GBuffer_Depth : register(t10);

TextureCube SkyMap : register(t11);
Texture2D SkySphere : register(t12);
Texture2D<float4> OffScreen : register(t13);

