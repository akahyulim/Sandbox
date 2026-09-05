#include "Resources.hlsli"
#include "Constants.hlsli"

static const float2 arrBasePos[4] =
{
    float2(-1.0, 1.0),
	float2(1.0, 1.0),
    float2(-1.0, -1.0),
	float2(1.0, -1.0),
};

struct VSToPS
{
    float4 position : SV_POSITION;
    float2 cpPos : TEXCOORD0;
};

VSToPS MainVS(uint VertexID : SV_VERTEXID)
{
    VSToPS output;

    output.position = float4(arrBasePos[VertexID].xy, 0.0, 1.0);
    output.cpPos = output.position.xy;

    return output;
}
/*
float4 MainPS(VSToPS input) : SV_Target
{
    float2 uv = input.cpPos * 0.5f + 0.5f;
    uv.y = 1.0f - uv.y; // 필요에 따라 제거
    
    // 텍스처 크기를 가져오거나 직접 정수 좌표로 변환
    uint width, height;
    ObjectIDMap.GetDimensions(width, height);
    uint2 texCoord = uint2(uv * float2(width, height));
    //uint2 texCoord = uint2(uv * frameData.screenResolution);
    
    // .Load()를 사용하면 정수 좌표로 정확하게 읽어올 수 있습니다 (추천)
    uint id = ObjectIDMap.Load(int3(texCoord, 0));
    
    return id == 0 ? float4(0.0f, 0.0f, 0.0f, 1.0f) : float4(1.0f, 1.0f, 1.0f, 1.0f);
    //return float4(float2(texCoord / frameData.screenResolution).xy, 1.0f, 1.0f);

}
*/
float4 MainPS(VSToPS input) : SV_Target
{
    float2 uv = input.cpPos * 0.5 + 0.5;
    uv.y = 1.0 - uv.y; // Y축 반전
    
    return OffScreen.Sample(ClampLinearSampler, uv);
}