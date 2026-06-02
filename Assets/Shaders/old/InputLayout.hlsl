// vertex
struct Vertex_PosColor
{
	float4 position : POSITION0;
	float4 color : COLOR0;
};

struct Vertex_PosTex
{
	float4 position : POSITION0;
	float2 tex : TEXCOORD0;
};

struct Vertex_PosTexNorTan
{
	float4 position : POSITION0;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL0;
	float3 tangent : TANGENT0;
};

struct Vertex_Skinned
{
	float4 position : POSITION0;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL0;
	float3 tangent : TANGENT0;
	uint4 boneIDs : BLENDINDICES;
	float4 boneWeights: BLENDWEIGHT;
};

// pixel
struct Pixel_PosColor
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
};

struct Pixel_PosTex
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

struct Pixel_PosTexNorTan
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL0;
	float3 tangent : TANGENT0;
};

struct Pixel_Test
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
};