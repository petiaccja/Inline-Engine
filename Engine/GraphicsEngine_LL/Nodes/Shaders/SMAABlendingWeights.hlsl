/*
* SMAA blending weights shader
* Input0: Edges color texture
* Input1: Area texture
* Input2: Search texture
* Output: Blending weights texture
*/

struct Uniforms
{
	float4 pixelSize;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

Texture2D edgesTex : register(t0); //Edges texture
Texture2D areaTex : register(t1); //Area texture
Texture2D searchTex : register(t2); //Search texture
SamplerState samp0 : register(s0);
SamplerState samp1 : register(s1);

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texcoord : TEX_COORD0;
	float2 pixcoord : TEX_COORD1;
	float4 offset[3] : TEX_COORD2;
};

#define SMAA_HLSL_4_1
#define SMAA_PRESET_ULTRA 1
#define SMAA_RT_METRICS uniforms.pixelSize

#include "SMAA"

PS_Input VSMain(float4 position : POSITION, float4 texcoord : TEX_COORD)
{
	PS_Input result;

	result.position = position;
	result.texcoord = texcoord.xy;

	SMAABlendingWeightCalculationVS(texcoord, result.pixcoord, result.offset);

	return result;
}

float4 PSMain(PS_Input input) : SV_TARGET
{
	return SMAABlendingWeightCalculationPS(input.texcoord, input.pixcoord, input.offset, edgesTex, areaTex, searchTex, float4(0, 0, 0, 0));
}
