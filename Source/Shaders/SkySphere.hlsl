#include "Constants.hlsli"
#include "Resources.hlsli"

struct VSToPS
{
    float4 Pos : SV_POSITION;
    float3 RayDir : TEXCOORD0; // 픽셀 단위로 정확하게 역산된 월드 시선 방향
};

VSToPS MainVS(uint vertexID : SV_VertexID)
{
    VSToPS output = (VSToPS) 0;

    // 1. SV_VertexID(0, 1, 2)를 이용해 화면 전체를 덮는 거대 삼각형 생성 (NDC 좌표)
    // Vertex 0: (-1, -1), Vertex 1: (-1, 3), Vertex 2: (3, -1)
    float2 grid = float2((vertexID == 2) ? 3.0f : -1.0f, (vertexID == 1) ? 3.0f : -1.0f);
    
    // 깊이값을 최원단(1.0f)으로 설정 (DepthStencilState::Skybox와 호환)
    output.Pos = float4(grid, 1.0f, 1.0f);

    // 2. NDC 화면 좌표(grid.x, grid.y, 1.0)를 월드 공간 위치로 역변환
    // * frameData.invViewProjection 행렬이 Constant Buffer에 전달되어 있어야 합니다.
    float4 unprojected = mul(float4(grid, 1.0f, 1.0f), frameData.inverseViewProjection);
    float3 worldPos = unprojected.xyz / unprojected.w;

    // 3. (월드 위치 - 카메라 위치)를 통해 픽셀별 정확한 시선 방향(Ray) 벡터 추출
    output.RayDir = worldPos - frameData.cameraPosition.xyz;

    return output;
}

// 방향 벡터를 2D 구면 파노라마 UV로 변환
float2 DirectionToSphericalUV(float3 dir)
{
    float3 normDir = normalize(dir);
    float u = atan2(normDir.z, normDir.x) * (1.0f / (2.0f * 3.14159265f)) + 0.5f;
    float v = asin(-normDir.y) * (1.0f / 3.14159265f) + 0.5f;
    return float2(u, v);
}

float4 MainPS(VSToPS input) : SV_Target
{
    // 픽셀별 구면 UV 계산
    float2 uv = DirectionToSphericalUV(input.RayDir);

    // 2D HDR 스키스피어 텍스처 샘플링 (AddressU = WRAP, AddressV = CLAMP 샘플러 적용)
    float3 color = SkySphere.Sample(SkySphereSampler, uv).rgb;

    return float4(color, 1.0f);
}