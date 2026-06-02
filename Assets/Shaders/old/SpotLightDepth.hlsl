#include "Common.hlsl"

cbuffer cbVertex : register(b2)
{
    matrix LightViewProj : packoffset(c0);
}

// 이걸로 Directional과 Point까지 커버할 수 없다.
// 즉, 파일 혹은 함수 이름으로 구분해야 한다.
void MainVS(float4 iPos : POSITION0,
            out float4 oPos : SV_POSITION)
{
    iPos.w = 1.0f;
    
    oPos = mul(iPos, ObjectWorld);
    oPos = mul(oPos, LightViewProj);
}