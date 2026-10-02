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

// 패치 상수 출력 구조체 (테셀레이션 배율 결정)
struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTess[3] : SV_TessFactor;
    float InsideTess : SV_InsideTessFactor;
};

// DS Output이자 PS Input으로 사용되는 구조체
struct DSToPS
{
    float4 PositionH : SV_POSITION; // 클립 공간 위치
    float2 UV : TEXCOORD0;
    float3 WorldPos : TEXCOORD1;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 BiNormal : BINORMAL;
};
/*
VSOutput MainVS(VSInput input)
{
    VSOutput output;
    
    float4 position = input.Position;
    position.w = 1.0f;
    
    output.PositionW = mul(position, objectData.model);
    output.PositionW = mul(output.PositionW, frameData.viewProjection);
    output.UV = input.UV;
    output.Normal = mul(input.Normal, (float3x3) objectData.model);
    output.Normal = normalize(output.Normal);
    output.Tangent = mul(input.Tangent, (float3x3) objectData.model);
    output.Tangent = normalize(output.Tangent);
    output.BiNormal = mul(input.BiNormal, (float3x3) objectData.model);
    output.BiNormal = normalize(output.BiNormal);
    
    return output;
}
*/
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
// =========================================================================
// 2. Hull Shader - Patch Constant Function (얼마나 잘게 쪼갤지 결정)
// =========================================================================
HS_CONSTANT_DATA_OUTPUT CalcPatchConstants(
    InputPatch<VSOutput, 3> patch,
    uint patchID : SV_PrimitiveID)
{
    HS_CONSTANT_DATA_OUTPUT output;

    // 1. 패치의 3개 정점 중점(Center) 계산 (또는 각 정점과 카메라 거리 계산)
    float3 patchCenter = (patch[0].PositionW.xyz + patch[1].PositionW.xyz + patch[2].PositionW.xyz) / 3.0f;

    // 2. 카메라 위치와의 거리 계산 (frameData 등에 카메라 월드 포지션이 있다고 가정)
    float distanceToCamera = distance(patchCenter, frameData.cameraPosition.xyz);

    // 3. 거리에 따른 테셀레이션 팩터 조절 (거리가 가까울수록 큼, 멀어지면 1.0으로 수렴)
    // 예: 최소 거리 5m 이내는 팩터 16, 최대 거리 50m 이상은 팩터 1 (테셀레이션 안 함)
    float minDistance = 5.0f;
    float maxDistance = 50.0f;
    
    // 거리에 비례하여 선형 또는 지수 함수로 보간 (saturate로 0~1 제한)
    float factor = 1.0f + 15.0f * saturate(1.0f - (distanceToCamera - minDistance) / (maxDistance - minDistance));

    output.EdgeTess[0] = factor;
    output.EdgeTess[1] = factor;
    output.EdgeTess[2] = factor;
    output.InsideTess = factor;

    return output;
}
/*
HS_CONSTANT_DATA_OUTPUT CalcPatchConstants(
    InputPatch<VSOutput, 3> patch,
    uint patchID : SV_PrimitiveID)
{
    HS_CONSTANT_DATA_OUTPUT output;

    // TODO: 카메라와의 거리에 따른 동적 테셀레이션 레벨 조절 가능 (우선 고정값 4.0f)
    float tessFactor = 4.0f; //16.0f;

    output.EdgeTess[0] = tessFactor;
    output.EdgeTess[1] = tessFactor;
    output.EdgeTess[2] = tessFactor;
    output.InsideTess = tessFactor;

    return output;
}
*/
// =========================================================================
// 3. Hull Shader - Control Point Function
// =========================================================================
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcPatchConstants")]
VSOutput MainHS(InputPatch<VSOutput, 3> patch, uint i : SV_OutputControlPointID)
{
    return patch[i];
}

// =========================================================================
// 4. Domain Shader (정점 분할 및 디스플레이스먼트 적용)
// =========================================================================
[domain("tri")]
DSToPS MainDS(
    HS_CONSTANT_DATA_OUTPUT inputConst,
    const OutputPatch<VSOutput, 3> patch,
    float3 barycentricCoords : SV_DomainLocation)
{
    DSToPS output;

    // 바리센트릭 좌표를 이용한 월드 포지션 보간
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

    // 디스플레이스먼트 적용 (월드 공간 기준)
    float2 uv = (output.UV * materialData.tiling) + materialData.offset;
    
    if (HasDisplacementMap())
    {
        float height = DisplacementMap.SampleLevel(WrapLinearSampler, uv, 0).r;
        output.WorldPos += output.Normal * (height * materialData.heightScale);
    }
    /*
    if (HasDisplacementMap())
    {
        float height = DisplacementMap.SampleLevel(WrapLinearSampler, uv, 0).r;
        
        // ★ 0~1 범위를 -0.5 ~ 0.5 범위로 변환 (0.5가 기본 평면 높이가 됨)
        float heightOffset = height - 0.5f;
        
        float heightScale = 0.2f; // 타일은 너무 높으면 깨지므로 0.2~0.3 정도로 낮춰서 테스트
        output.WorldPos += output.Normal * (heightOffset * heightScale);
    }
*/
    // ★ 여기서 딱 한 번만 뷰-프로젝션 행렬 곱하여 클립 공간(SV_POSITION)으로 변환
    output.PositionH = mul(float4(output.WorldPos, 1.0f), frameData.viewProjection);

    return output;
}
/*
[domain("tri")]
DSToPS MainDS(
    HS_CONSTANT_DATA_OUTPUT inputConst,
    const OutputPatch<VSOutput, 3> patch,
    float3 barycentricCoords : SV_DomainLocation)
{
    DSToPS output;

    // 바리센트릭 좌표를 이용한 속성 보간
    output.WorldPos = patch[0].PositionW.xyz * barycentricCoords.x +
                      patch[1].PositionW.xyz * barycentricCoords.y +
                      patch[2].PositionW.xyz * barycentricCoords.z;

    output.UV = patch[0].UV * barycentricCoords.x +
                patch[1].UV * barycentricCoords.y +
                patch[2].UV * barycentricCoords.z;

    output.Normal = patch[0].Normal * barycentricCoords.x +
                    patch[1].Normal * barycentricCoords.y +
                    patch[2].Normal * barycentricCoords.z;
    output.Normal = normalize(output.Normal);

    output.Tangent = patch[0].Tangent * barycentricCoords.x +
                     patch[1].Tangent * barycentricCoords.y +
                     patch[2].Tangent * barycentricCoords.z;
    output.Tangent = normalize(output.Tangent);

    output.BiNormal = patch[0].BiNormal * barycentricCoords.x +
                      patch[1].BiNormal * barycentricCoords.y +
                      patch[2].BiNormal * barycentricCoords.z;
    output.BiNormal = normalize(output.BiNormal);

    // ★ [핵심] 타일링 및 오프셋 계산 후 디스플레이스먼트 맵(Height) 적용
    float2 uv = (output.UV * materialData.tiling) + materialData.offset;
    
    if (HasDisplacementMap())
    {
        // 0~1 범위를 높이 값으로 샘플링 (Domain Shader 스테이지에서는 SampleLevel 사용)
        float height = DisplacementMap.SampleLevel(WrapLinearSampler, uv, 0).r;
        
        // 노멀 방향으로 지형/오브젝트를 밀어냄 (materialData에 heightScale이 있다고 가정, 없으면 상수로 조절)
        float heightScale = 0.05f;
        output.WorldPos += output.Normal * (height * heightScale);
    }

    // 최종 뷰-프로젝션 행렬 곱하여 클립 공간으로 변환
    output.PositionH = mul(float4(output.WorldPos, 1.0f), frameData.viewProjection);

    return output;
}
*/
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
    
    // Albedo
    float3 albedo = materialData.baseColor;
    if (HasAlbedoMap())
    {
        albedo *= AlbedoMap.Sample(WrapLinearSampler, uv).xyz;
        albedo *= albedo; // Gamma to Linear correction 예시
    }
    
    // Roughness 및 Metallic 기본값 설정
    float roughness = materialData.roughnessFactor;
    float metallic = materialData.metallicFactor;

    // ORM 맵 반영 (R: AO, G: Roughness, B: Metallic)
    if (HasORMMap())
    {
        float3 orm = ORMMap.Sample(WrapLinearSampler, uv).xyz;
        //orm.r (AO는 필요에 따라 Ambient Occlusion 렌더 타겟에 쓸 수 있음)
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
    
    // Normal 맵 적용 (테셀레이션으로 찌그러진 지오메트리 위에 노멀 디테일 추가)
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