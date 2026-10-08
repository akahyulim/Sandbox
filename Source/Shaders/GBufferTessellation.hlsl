#include "Constants.hlsli"
#include "Resources.hlsli"

struct VSInput
{
    float4 Position : POSITION;
    float2 UV : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 BiNormal : BINORMAL;
};

struct VSOutput
{
    float4 PositionW : SV_POSITION;
    float2 UV : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 BiNormal : BINORMAL;
};

struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTess[3] : SV_TessFactor;
    float InsideTess : SV_InsideTessFactor;
};

struct DSToPS
{
    float4 PositionH : SV_POSITION;
    float2 UV : TEXCOORD0;
    float3 WorldPos : TEXCOORD1;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 BiNormal : BINORMAL;
};

VSOutput MainVS(VSInput input)
{
    VSOutput output;
    
    float4 position = input.Position;
    position.w = 1.0f;
    
    // 월드 좌표까지만 계산
    output.PositionW = mul(position, objectData.model);
    output.UV = input.UV;
    output.Normal = mul(input.Normal, (float3x3) objectData.model);
    output.Normal = normalize(output.Normal);
    output.Tangent = mul(input.Tangent, (float3x3) objectData.model);
    output.Tangent = normalize(output.Tangent);
    output.BiNormal = mul(input.BiNormal, (float3x3) objectData.model);
    output.BiNormal = normalize(output.BiNormal);
    
    return output;
}


HS_CONSTANT_DATA_OUTPUT CalcPatchConstants(
    InputPatch<VSOutput, 3> patch,
    uint patchID : SV_PrimitiveID)
{
    HS_CONSTANT_DATA_OUTPUT output;

    float3 patchCenter = (patch[0].PositionW.xyz + patch[1].PositionW.xyz + patch[2].PositionW.xyz) / 3.0f;

    float distanceToCamera = distance(patchCenter, frameData.cameraPosition.xyz);

    float minDistance = 5.0f;
    float maxDistance = 50.0f;
    
    float factor = 1.0f + 15.0f * saturate(1.0f - (distanceToCamera - minDistance) / (maxDistance - minDistance));

    output.EdgeTess[0] = factor;
    output.EdgeTess[1] = factor;
    output.EdgeTess[2] = factor;
    output.InsideTess = factor;

    return output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcPatchConstants")]
VSOutput MainHS(InputPatch<VSOutput, 3> patch, uint i : SV_OutputControlPointID)
{
    return patch[i];
}

[domain("tri")]
DSToPS MainDS(
    HS_CONSTANT_DATA_OUTPUT inputConst,
    const OutputPatch<VSOutput, 3> patch,
    float3 barycentricCoords : SV_DomainLocation)
{
    DSToPS output;
    
    output.WorldPos = patch[0].PositionW.xyz * barycentricCoords.x +
                      patch[1].PositionW.xyz * barycentricCoords.y +
                      patch[2].PositionW.xyz * barycentricCoords.z;

    output.UV = patch[0].UV * barycentricCoords.x +
                patch[1].UV * barycentricCoords.y +
                patch[2].UV * barycentricCoords.z;

    output.Normal = normalize(patch[0].Normal * barycentricCoords.x +
                              patch[1].Normal * barycentricCoords.y +
                              patch[2].Normal * barycentricCoords.z);

    output.Tangent = normalize(patch[0].Tangent * barycentricCoords.x +
                               patch[1].Tangent * barycentricCoords.y +
                               patch[2].Tangent * barycentricCoords.z);

    output.BiNormal = normalize(patch[0].BiNormal * barycentricCoords.x +
                                patch[1].BiNormal * barycentricCoords.y +
                                patch[2].BiNormal * barycentricCoords.z);

    float2 uv = (output.UV * materialData.tiling) + materialData.offset;
    
    if (HasDisplacementMap())
    {
        float height = DisplacementMap.SampleLevel(WrapLinearSampler, uv, 0).r;
        output.WorldPos += output.Normal * (height * materialData.heightScale);
    }

    output.PositionH = mul(float4(output.WorldPos, 1.0f), frameData.viewProjection);

    return output;
}

struct PSOutput
{
    float4 AlbedoRoughness : SV_TARGET0;
    float4 NormalMetallic : SV_TARGET1;
    float4 Emissive : SV_TARGET2;
    uint ObjectID : SV_TARGET3;
};

PSOutput MainPS(DSToPS input)
{
    PSOutput output;
    
    float2 uv = (input.UV * materialData.tiling) + materialData.offset;
    
    float3 albedo = materialData.baseColor;
    if (HasAlbedoMap())
    {
        albedo *= AlbedoMap.Sample(WrapLinearSampler, uv).xyz;
        albedo *= albedo; // Gamma to Linear correction 예시
    }
    
    float roughness = materialData.roughnessFactor;
    float metallic = materialData.metallicFactor;

    if (HasORMMap())
    {
        float3 orm = ORMMap.Sample(WrapLinearSampler, uv).xyz;
        roughness *= orm.g;
        metallic *= orm.b;
    }
    else
    {
        if (HasRoughnessMap())
        {
            float value = RoughnessMap.Sample(WrapLinearSampler, uv).r;
            roughness *= value;
        }
        
        if (HasMetallicMap())
        {
            float value = MetallicMap.Sample(WrapLinearSampler, uv).r;
            metallic *= value;
        }
    }

    output.AlbedoRoughness = float4(albedo, roughness);
    
    float3 normal = input.Normal;
    if (HasNormalMap())
    {
        float4 bumpMap = NormalMap.Sample(WrapLinearSampler, uv);
        bumpMap = (bumpMap * 2.0f) - 1.0f;

        normal = normalize((bumpMap.x * input.Tangent) + (bumpMap.y * input.BiNormal) + (bumpMap.z * input.Normal));
    }
    
    output.NormalMetallic = float4(normal * 0.5f + 0.5f, metallic);
    
    output.Emissive = float4(0.0f, 0.0f, 0.0f, 0.0f);
    output.ObjectID = objectData.id;
    
    return output;
}