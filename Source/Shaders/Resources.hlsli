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
Texture2D ORMMap : register(t2);            // Ambient Occlusion (R), Roughness (G), Metallic (B) 통합 맵
Texture2D DisplacementMap : register(t3);   // 테셀레이션 및 디스플레이스먼트용 높이 맵
Texture2D EmissiveMap : register(t4);
Texture2D OpacityMap : register(t5);

// G-Buffer Inputs (t6 ~ t9)
Texture2D<float4> GBuffer_AlbedoRoughness : register(t6);   // RGB: Albedo(기본 색상) / A: Roughness(거친 정도)
Texture2D<float4> GBuffer_NormalMetallic : register(t7);    // RGB: View Space Normal(법선) / A: Metallic(금속성)
Texture2D<float4> GBuffer_Emissive : register(t8);          // RGB: Emissive(자체 발광 색상)
Texture2D<uint> GBuffer_ObjectID : register(t9);            // 추후 슬롯 맞추기
Texture2D<float> GBuffer_Depth : register(t10);             // Depth (깊이 버퍼 - 월드 좌표 복원용)

TextureCube SkyMap : register(t11);
Texture2D SkySphere : register(t12);
Texture2D<float4> OffScreen : register(t13);

