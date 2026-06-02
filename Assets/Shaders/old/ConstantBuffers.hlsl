// 현재 사용하지 않지만 Skinning 등에 활용되었기에 참고에 필요할 수 있다.


// urho3d 기준
// frame, camera, zone, light, material 총 5개 + objectVS로 구성
// 기본적으로 vs와 ps의 구성이 다르지만, 같거나 중복전달하는 경우도 존재
// map/unmap은 batch에서 개별 요소마다 업데이트
// bind는 Graphics::SetShaders() 내부에서 한 번에 실행

// 이렇게 데이터마다 개별 이름을 두는 것 보다
// 구조체로 만들고 다시 cbuffer로 선언하는 편이 나아보인다.

//#ifdef COMPILEVS

// FrameVS

// CameraVS : 여기에 cViewProj로 view와 proj의 곱을 저장하는 듯 하다.
cbuffer CameraVS : register(b0)
{
	//float3 cbCameraPosVS;
	matrix cbCameraMatrixVS;
	matrix cbViewMatrixVS;
	matrix cbProjMatrixVS;
}

// LightVS
// ObjectVS : 여기에 worldTransform이 cModel이름으로 전달된다.
cbuffer ObjectVS : register(b1)
{
	matrix cbWorldMatrixVS;

	matrix cbSkinMatrix[100];
}

//#endif

//#ifdef COMPILEPS

// FramePS
// CameraPS
cbuffer CameraPS : register(b0)
{
	float3 cbCameraPosPS;	// 이름이 같으면 안된다.
	float cameraPoo;
	float4 cbPerspectiveValuePS;
	matrix cbViewInvPS;
}

// LightPS
cbuffer LightPS : register(b1)
{
	float3 cbLightPosPS;
	float cbLightRangePS;
	float3 cbLightColorPS;
	float cbLightSpotAnglePS;
	float3 cbLightDirPS;
	float lightPoo;
	//float lightIntensity;	// 이건 material 특성인 듯 하다.
}

// MaterialPS
cbuffer MaterialPS : register(b2)
{
	float4 cbMaterialAlbedoColorPS;

	float2 cbMaterialTilingPS;
	float2 cbMaterialOffsetPS;

	uint cbProperties;
	float3 padding;
}

bool has_texture_diffse_old()                     { return cbProperties & uint(1U << 0); }

//#endif