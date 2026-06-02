// samplers
SamplerState WrapLinearSampler : register(s0);
SamplerState ClampPointSampler : register(s1);
SamplerState ClampLinearSampler : register(s2);
SamplerState SkyboxSampler : register(s2);
SamplerState ShadowCompare : register(s4);

// Material Textures (t0 ~ t5)
Texture2D DiffuseMap : register(t0);
Texture2D NormalMap : register(t1);
Texture2D RoughnessMap : register(t2);
Texture2D MetallicMap : register(t3);
Texture2D EmissiveMap : register(t4);
Texture2D OpacityMap : register(t5);

// G-Buffer Inputs (t10 ~ t15)
Texture2D<float4> GBuffer_Albedo : register(t10);
Texture2D<float3> GBuffer_Normal : register(t11);
Texture2D<float2> GBuffer_RoughMetal : register(t12);
Texture2D<float4> GBuffer_Emissive : register(t13);
Texture2D<float> GBuffer_Depth : register(t14);
Texture2D<uint> GBuffer_MaterialID : register(t15);

// Shadow & Lighting Maps (t20 ~ t25)
Texture2D<float> ShadowMap : register(t20);
TextureCube Skymap : register(t21);
TextureCube<float4> EnvironmentMap : register(t22);
TextureCube<float3> IBL_Irradiance : register(t23);
TextureCube<float3> IBL_Specular : register(t24);
Texture2D<float2> BRDF_LUT : register(t25);

// Post-Processing Inputs (t30 ~ t33)
Texture2D<float4> SceneColor : register(t30);
Texture2D<float> SceneDepth : register(t31);
Texture2D<float4> BloomTexture : register(t32);
Texture2D<float2> MotionVector : register(t33);