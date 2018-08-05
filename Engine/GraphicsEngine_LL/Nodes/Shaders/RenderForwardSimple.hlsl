struct VsConstants
{
    float4x4 world;
    float4x4 worldViewProj;
    float4x4 worldViewProjDer;
};
ConstantBuffer<VsConstants> vsConstants : register(b0);

struct PsInput
{
    float4 hPos : SV_Position;
    float4 wPos : TEXCOORD10;
    float2 sVelocity : VELOCITY;
#ifdef HAS_NORMAL
	float3 wNormal : NORMAL;
#endif
#ifdef HAS_COLOR
	float4 color : COLOR0;
#endif
#ifdef HAS_TEXCOORD
	float2 texCoord : TEX_COORD0;
#endif
#ifdef HAS_TANGENT
	float3 wTangent : TANGENT;
	float3 wBitangent : BITANGENT;
#endif
};


PsInput VSMain(float4 lPos : POSITION,
#ifdef HAS_NORMAL
				float3 lNormal : NORMAL,
#endif
#ifdef HAS_COLOR
				float4 color : COLOR,
#endif
#ifdef HAS_TEXCOORD
				float2 texCoord : TEX_COORD,
#endif
#ifdef HAS_TANGENT
				float3 lTangent : TANGENT,
#endif
#ifdef HAS_BITANGENT
				float3 lBitangent : BITANGENT
#endif
)
{
    PsInput output;

    output.hPos = mul(lPos, vsConstants.worldViewProj);
    output.sVelocity = mul(lPos, vsConstants.worldViewProjDer).xy;
    output.wPos = mul(lPos, vsConstants.world);

#ifdef HAS_NORMAL
    float3x3 worldRotation = float3x3(vsConstants.world);
    output.wNormal = mul(lNormal, worldRotation);
#endif
#ifdef HAS_COLOR
    output.color = color;
#endif
#ifdef HAS_TEXCOORD
    output.texCoord = texCoord;
#endif
#ifdef HAS_TANGENT
    output.wTangent = mul(lTangent, worldRotation);
	#if HAS_BITANGENT
		output.wBitangent = mul(lBitangent, worldRotation);
	#else 
		output.wBitangent = cross(output.wNormal, output.wBitangent);
	#endif
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
static float4 go_color;


// Implementations
#define NORMAL_IMPLEMENTATION return go_wNormal;
#define TANGENT_IMPLEMENTATION return go_wTangent;
#define BITANGENT_IMPLEMENTATION return go_wBitangent;
#define COLOR_IMPLEMENTATION return g_vertexColor;
#define TEXCOORD_IMPLEMENTATION return g_texCoord;

#define LIGHTCOUNT_IMPLEMENTATION return 1;
#define LIGHTDIR_IMPLEMENTATION return psConstants.lightDir;
#define LIGHTCOLOR_IMPLEMENTATION return psConstants.lightColor;

#define SINK_IMPLEMENTATION go_color = color;

struct MapColor2D {
    Texture2DArray<float4> tex;
    SamplerState samp;
};
struct MapValue2D {
    Texture2DArray<float> tex;
    SamplerState samp;
};


//!1#include "material_shader_################.hlsl"


PsOutput PSMain(PsInput input)
{
    g_wPosition = input.wPos;

#if HAS_NORMAL
    g_wNormal = input.wNormal;
#endif

#if HAS_TANGENT
    g_wTangent = input.wTangent;
    g_wBitangent = input.wBitangent;
#endif

#if HAS_COLOR
    g_vertexColor = input.color;
#endif

#if HAS_TEXCOORD
    g_texCoord = input.texCoord;
#endif

    PsOutput output;

    MtlMain();

	output.color = go_color;
}
