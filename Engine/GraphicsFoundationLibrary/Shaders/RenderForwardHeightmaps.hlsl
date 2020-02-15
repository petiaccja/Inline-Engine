//------------------------------------------------------------------------------
// Constant buffers
//------------------------------------------------------------------------------

struct VsConstants
{
	float4x4 world;
	float4x4 viewProj;
	float4x4 worldViewProjDer;
	float3 direction;
	float _padding01;
	float magnitude;
	float offset;
	float2 uvSize;
	float2 screenSize;
};
ConstantBuffer<VsConstants> vsConstants : register(b0);


struct PsConstants
{
	float3 lightDir;
	float3 lightColor;
};
ConstantBuffer<PsConstants> psConstants : register(b100);


SamplerState heightmapSampler : register(s0);
Texture2D<float4> heightmapTex : register(t0);


//------------------------------------------------------------------------------
// Helper functions
//------------------------------------------------------------------------------

struct Derivatives
{
	float z;
	float2 grad;
};


float2 CalculateDuv(Texture2D tex, float2 texCoord, float2 triangleUvSize)
{
	float width;
	float height;
	uint numLevels;
	tex.GetDimensions(0, width, height, numLevels);
	float2 pixelMin = 1.0f / float2(width, height) * 0.5f;
	float2 target = length(triangleUvSize) * 1.0f;
	return max(pixelMin, target);
}


Derivatives SampleHeightmap(Texture2D tex, SamplerState samp, float2 texCoord, float2 uvSize, float2 duv)
{
	Derivatives result;
	result.z = vsConstants.magnitude * heightmapTex.SampleLevel(heightmapSampler, texCoord, 0);
	
	float2 dz = float2(
		vsConstants.magnitude * heightmapTex.SampleLevel(heightmapSampler, texCoord + float2(duv.x, 0), 0).x - result.z,
		vsConstants.magnitude * heightmapTex.SampleLevel(heightmapSampler, texCoord + float2(0, duv.y), 0).x - result.z
	);
	float2 dz2 = float2(
		result.z - vsConstants.magnitude * heightmapTex.SampleLevel(heightmapSampler, texCoord + float2(-duv.x, 0), 0).x,
		result.z - vsConstants.magnitude * heightmapTex.SampleLevel(heightmapSampler, texCoord + float2(0, -duv.y), 0).x
	);
	
	float2 duvDxy = float2(1.0f, 1.0f) / uvSize;
	
	float2 dzDxy = (dz + dz2) / 2.0f / duv * duvDxy;
	
	result.grad = dzDxy;
	return result;
}


float3 DisplacementAdjustedNormal(float3 normal, float3 tangent, float3 bitangent, float2 dnDtb)
{
	float3 gradT = tangent + normal * dnDtb.x;
	float3 gradB = bitangent + normal * dnDtb.y;
	return cross(gradT, gradB);
}


//------------------------------------------------------------------------------
// Material rendering implementation
//------------------------------------------------------------------------------

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
#ifndef PIXEL_SHADER
void MtlMain()
{
}
#endif


//------------------------------------------------------------------------------
// Transfer structures
//------------------------------------------------------------------------------

struct VsOutput
{
	float4 lPos : POSITION;
	float4 lPosDisplaced : POSITIOND;
	float3 lNormal : NORMAL;
	float3 lTangent : TANGENT;
	float2 texCoord : TEX_COORD;
	float3 lBitangent : BITANGENT;
#ifdef HAS_COLOR
	float4 color : COLOR;
#endif
};

struct HsOutput
{
	float4 lPos : POSITION;
	float3 lNormal : NORMAL;
	float3 lTangent : TANGENT;
	float2 texCoord : TEX_COORD;
	float3 lBitangent : BITANGENT;
#ifdef HAS_COLOR
	float4 color : COLOR;
#endif
};

struct HsConstantDataOutput
{
	float edges[3] : SV_TessFactor;
	float inside : SV_InsideTessFactor;
};

struct DsOutput
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

struct PsOutput
{
	float4 color : SV_Target0;
};


//------------------------------------------------------------------------------
// Vertex shader
//------------------------------------------------------------------------------
VsOutput VSMain(float4 lPos : POSITION,
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
	// Calculate bitangent if not provided.
#if !HAS_BITANGENT
	float3 lBitangent = cross(lNormal, lTangent);
#endif
	
	VsOutput output;
	
	output.lPos = lPos;
	output.lNormal = lNormal;
	output.lTangent = lTangent;
	output.lBitangent = lBitangent;
	output.texCoord = texCoord;
#ifdef HAS_COLOR
	output.color = color;
#endif
	
	float z = vsConstants.magnitude * heightmapTex.SampleLevel(heightmapSampler, texCoord, 0);
	output.lPosDisplaced = lPos + float4(vsConstants.direction * (z + vsConstants.offset), 0);

	return output;
}

//------------------------------------------------------------------------------
// Hull shader
//------------------------------------------------------------------------------

HsConstantDataOutput Subdivide(InputPatch<VsOutput, 3> patch, uint patchId : SV_PrimitiveID)
{
	float4x4 wvp = mul(vsConstants.world, vsConstants.viewProj);
	float3 scPos[3];
	for (uint i = 0; i < 3; ++i)
	{
		float4 hPos = mul(patch[i].lPosDisplaced, wvp);
		scPos[i] = hPos.xyz / hPos.w;
	}
	//bool cullz = scPos[0].z < -1.0f && scPos[1].z < -1.0f && scPos[2].z < -1.0f;
	bool cullx = (scPos[0].x > 1.0f && scPos[1].x > 1.0f && scPos[2].x > 1.0f)
				|| (scPos[0].x < -1.0f && scPos[1].x < -1.0f && scPos[2].x < -1.0f);
	bool cully = (scPos[0].y > 1.0f && scPos[1].y > 1.0f && scPos[2].y > 1.0f)
				|| (scPos[0].y < -1.0f && scPos[1].y < -1.0f && scPos[2].y < -1.0f);
	
	HsConstantDataOutput output;
	if (cullx || cully)
	{
		output.edges[0] = 0.0f;
		output.edges[1] = 0.0f;
		output.edges[2] = 0.0f;
		output.inside = 0.0f;
	}
	else
	{
		output.inside = 0.0f;
		for (uint i = 0; i < 3; ++i)
		{
			float len = length((scPos[(i + 1) % 3].xy - scPos[(i + 2) % 3].xy) * vsConstants.screenSize);
			output.edges[i] = len / 32.f;
			output.inside += output.edges[i];
		}
		output.inside /= 3.0f;
	}
	
	return output;
}


[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("Subdivide")]
HsOutput HSMain(InputPatch<VsOutput, 3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
	HsOutput output;
	
	output.lPos = patch[pointId].lPos;
	output.lNormal = patch[pointId].lNormal;
	output.lTangent = patch[pointId].lTangent;
	output.lBitangent = patch[pointId].lBitangent;
	output.texCoord = patch[pointId].texCoord;
#ifdef HAS_COLOR
	output.color = patch[pointId].color;
#endif
	return output;
}

//------------------------------------------------------------------------------
// Domain shader
//------------------------------------------------------------------------------

[domain("tri")]
DsOutput DSMain(HsConstantDataOutput constantInput, float3 bariCoord : SV_DomainLocation, const OutputPatch<HsOutput, 3> patch)
{
	HsOutput interpolated;
	
	interpolated.lPos = bariCoord[0] * patch[0].lPos + bariCoord[1] * patch[1].lPos + bariCoord[2] * patch[2].lPos;
	interpolated.lNormal = bariCoord[0] * patch[0].lNormal + bariCoord[1] * patch[1].lNormal + bariCoord[2] * patch[2].lNormal;
	interpolated.lTangent = bariCoord[0] * patch[0].lTangent + bariCoord[1] * patch[1].lTangent + bariCoord[2] * patch[2].lTangent;
	interpolated.lBitangent = bariCoord[0] * patch[0].lBitangent + bariCoord[1] * patch[1].lBitangent + bariCoord[2] * patch[2].lBitangent;
	interpolated.texCoord = bariCoord[0] * patch[0].texCoord + bariCoord[1] * patch[1].texCoord + bariCoord[2] * patch[2].texCoord;
#ifdef HAS_COLOR
	interpolated.color = bariCoord[0] * patch[0].color + bariCoord[1] * patch[1].color + bariCoord[2] * patch[2].color;
#endif	
	
	DsOutput output;
	
	float2 maxTexCoord = max(patch[0].texCoord, max(patch[1].texCoord, patch[2].texCoord));
	float2 minTexCoord = min(patch[0].texCoord, min(patch[1].texCoord, patch[2].texCoord));
	float2 triangleUvSize = (maxTexCoord - minTexCoord) / max(1.0f, constantInput.inside);
	float2 duv = CalculateDuv(heightmapTex, interpolated.texCoord, triangleUvSize);
	Derivatives derivatives = SampleHeightmap(heightmapTex, heightmapSampler, interpolated.texCoord, vsConstants.uvSize, duv);
	
	float4 modifiedPos = interpolated.lPos + float4(vsConstants.direction * (derivatives.z + vsConstants.offset), 0);
	float3 modifiedNormal = DisplacementAdjustedNormal(interpolated.lNormal, interpolated.lTangent, interpolated.lBitangent, derivatives.grad);

	// Transform position.
	output.sVelocity = mul(modifiedPos, vsConstants.worldViewProjDer).xy;
	output.wPos = mul(modifiedPos, vsConstants.world);
	output.hPos = mul(output.wPos, vsConstants.viewProj);
	
	// Write through texCoord.
	output.texCoord = interpolated.texCoord;
	
	// Transform through normal.
	float3x3 worldRotation = (float3x3) vsConstants.world;
	output.wNormal = normalize(mul(modifiedNormal, worldRotation));
	
	// Transform through tangent.
	output.wTangent = mul(interpolated.lTangent, worldRotation);
	
	// Transform through bitangent.
	output.wBitangent = mul(interpolated.lBitangent, worldRotation);
	
#ifdef HAS_COLOR
	output.color = color;
#endif
	return output;
}

//------------------------------------------------------------------------------
// Pixel shader
//------------------------------------------------------------------------------
PsOutput PSMain(DsOutput input)
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
