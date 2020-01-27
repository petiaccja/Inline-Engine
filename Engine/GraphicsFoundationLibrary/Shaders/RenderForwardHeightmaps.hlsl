struct VsConstants
{
	float4x4 world;
	float4x4 viewProj;
	float4x4 worldViewProjDer;
	float3 direction;
	float magnitude;
	float offset;
};
ConstantBuffer<VsConstants> vsConstants : register(b0);

SamplerState heightmapSampler : register(s0);
Texture2D<float4> heightmapTex : register(t0);

struct PsInput
{
	float4 hPos : SV_Position;
	float4 wPos : Output0;
	float2 sVelocity : Output1;
	float3 wNormal : Output2;
	float2 texCoord : TEXCOORD0;
	float3 wTangent : Output4;
	float3 wBitangent : Output5;
#ifdef HAS_COLOR
	float4 color : Output3;
#endif
};


PsInput VSMain(float4 lPos : POSITION,
				 float3 lNormal : NORMAL,
				 float3 lTangent : TANGENT,
				 float2 texCoord : TEX_COORD
#ifdef HAS_COLOR
				 , float4 color : COLOR
#endif
#ifdef HAS_BITANGENT
				 , float3 lBitangent : BITANGENT
#endif
)
{
	PsInput output;

	output.sVelocity = mul(lPos, vsConstants.worldViewProjDer).xy;
	output.wPos = mul(lPos, vsConstants.world);
		
	float z = heightmapTex.SampleLevel(heightmapSampler, texCoord, 0);
	output.wPos.xyz += vsConstants.direction * (z * vsConstants.magnitude + vsConstants.offset);
	output.hPos = mul(output.wPos, vsConstants.viewProj);
	
	float du = 0.01f;
	float dv = 0.01f;
	float zx = heightmapTex.SampleLevel(heightmapSampler, texCoord + float2(du, 0), 0);
	float zy = heightmapTex.SampleLevel(heightmapSampler, texCoord + float2(0, dv), 0);
	
	float dudx = 1.0f;
	float dvdy = 1.0f;
	
	float dzdx = (zx - z) / (du / dudx);
	float dzdy = (zy - z) / (dv / dvdy);
	
	float3 gradX = float3(1.0f, 0.0f, dzdx);
	float3 gradY = float3(0.0f, 1.0f, dzdy);
	float3 normal = cross(gradX, gradY);

	// Write through texCoord.
	output.texCoord = texCoord;
	
	// Transform through normal.
	float3x3 worldRotation = (float3x3)vsConstants.world;
	output.wNormal = normalize(mul(normal, worldRotation));
	
	// Transform through tangent.
	output.wTangent = mul(lTangent, worldRotation);
	
	// Transform through or calculate bitangent.
#if HAS_BITANGENT
	output.wBitangent = mul(lBitangent, worldRotation);
#else
	output.wBitangent = cross(output.wNormal, output.wTangent);
#endif
	
	#ifdef HAS_COLOR
	output.color = color;
#endif

	return output;
}



struct PsConstants
{
	float3 lightDir;
	float3 lightColor;
};
ConstantBuffer<PsConstants> psConstants : register(b100);


struct PsOutput
{
	float4 color : SV_Target0;
};


// Surface params
static float3 g_wPosition;
static float3 g_wNormal;
static float3 g_wTangent;
static float3 g_wBitangent;
static float4 g_vertexColor;
static float2 g_texCoord;


// GBuffer output
static float4 go_color = float4(1, 0, 0, 1);


// Implementations
#define NORMAL_IMPLEMENTATION return g_wNormal;
#define TANGENT_IMPLEMENTATION return g_wTangent;
#define BITANGENT_IMPLEMENTATION return g_wBitangent;
#define COLOR_IMPLEMENTATION return g_vertexColor;
#define TEXCOORD_IMPLEMENTATION return g_texCoord;

#define LIGHTCOUNT_IMPLEMENTATION return 1;
#define LIGHTDIR_IMPLEMENTATION return psConstants.lightDir;
#define LIGHTCOLOR_IMPLEMENTATION return psConstants.lightColor;

#define SINK_IMPLEMENTATION go_color = color;


#define main main_sf
#include "SurfaceInputsFunc.hlsl"
#define main main_li
#include "LightInputs.hlsl"
#define main main_tp
#include "Types.hlsl"
#undef main


//MATERIAL_CODE_INCLUDE
#ifdef VERTEX_SHADER
void MtlMain() {}
#endif


PsOutput PSMain(PsInput input)
{
	g_wPosition = input.wPos;
	g_wNormal = normalize(input.wNormal);
	g_wTangent = normalize(input.wTangent);
	g_wBitangent = normalize(input.wBitangent);
	g_texCoord = input.texCoord;

#ifdef HAS_COLOR
	g_vertexColor = input.color;
#endif

	PsOutput output;

	MtlMain();

	output.color = go_color;
	return output;
}
